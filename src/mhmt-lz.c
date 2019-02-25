#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mhmt-types.h"

#include "mhmt-lz.h"
#include "mhmt-tb.h"
#include "mhmt-globals.h"



// Universal search function
//
void make_lz_codes(OFFSET position, ULONG actual_len, UBYTE * hash, struct lzcode * codes)
{
	OFFSET i;
	ULONG codepos;
	ULONG codelen;
	ULONG was_match;
	UBYTE curr_byte,next_byte;
	struct tb_chain * curr_tb;
	UWORD index;
	ULONG max_lookback,max_length,max_tbdisp;

	// copy-byte code is always present
	codes[0].length = 1;
	codes[0].disp   = 0;

	// start more filling of codes[] from that position
	codepos = 1;


	
	if( wrk.packtype==PK_HST ) // for hrust only,
	{                          // add 12,14,16,...,40,42 bytes copies, if possible
		for(codelen=12;codelen<=42;codelen+=2)
		{
			if( position > (OFFSET)(actual_len-codelen) )
				break;
		
			codes[codepos].length = codelen;
			codes[codepos].disp   = 0;
			codepos++;
		}
	}




	curr_byte=wrk.indata[position];



	if( wrk.packtype!=PK_ZX7 )
	{
		// check for one-byter (-1..-8)
		//
		i = (position > (8LL-wrk.prelen) ) ? position-8 : (0LL-wrk.prelen);
		do
		{
			if( wrk.indata[i] == curr_byte )
			{
				codes[codepos].length = 1;
				codes[codepos].disp   = -(LONG)(position-i);
				codepos++;
				break;
			}
		} while( (++i)<position );
	}





	// for hrust, check 3-byte insertion code (-1..-79)
	if( (wrk.packtype==PK_HST) && (position < (OFFSET)(actual_len-2)) )
	{
		i = (position > (79LL-wrk.prelen) ) ? position-79 : (0LL-wrk.prelen);
		do
		{
			if( (wrk.indata[i]==curr_byte) && (wrk.indata[i+2]==wrk.indata[position+2]) )
			{
				codes[codepos].length = (-3);
				codes[codepos].disp   = -(LONG)(position-i);
				codepos++;
				break;
			}
		} while( (++i)<position );
	}



	
	switch( wrk.packtype ) // set maximum lookback and length
	{
	case PK_MLZ:
		max_lookback = (wrk.maxwin<4352) ? wrk.maxwin : 4352;
		max_length = 255;
		max_tbdisp = 256;
		break;
	case PK_HRM:
		max_lookback = (wrk.maxwin<4096) ? wrk.maxwin : 4096;
		max_length = 255;
		max_tbdisp = 256;
		break;
	case PK_HST:
		max_lookback = (wrk.maxwin<65536) ? wrk.maxwin : 65536;
		max_length = 3839;
		max_tbdisp = 768;
		break;
	case PK_ZX7:
		max_lookback = (wrk.maxwin<2176) ? wrk.maxwin : 2176;
		max_length = 65536;
		max_tbdisp = max_lookback;
		break;
	default:
		printf("mhmt-lz.c:make_lz_codes() - wrong packer type!\n");
		exit(1);
	}



	// check for two-byter (-1..-max_tbdisp)
	//
	curr_tb = NULL;
	//
	if( position<(OFFSET)(actual_len-1) ) // don't try two-byter if we are at the byte before last one
	{
		next_byte = wrk.indata[position+1];
		index=(curr_byte<<8) + next_byte;
		curr_tb = tb_entry[index];

		// there are two-byters!
		if( curr_tb )
		{
			if( ((position-curr_tb->pos)<=(OFFSET)max_tbdisp) && ((position-curr_tb->pos)<=(OFFSET)max_lookback) )
			{
				codes[codepos].length = 2;
				codes[codepos].disp   = -(LONG)(position - curr_tb->pos);
				codepos++;
			}
		}
	}


	// at last, check for lengths=3..max_length up to max_lookback 
	if(  curr_tb  &&  ( (position-curr_tb->pos)<=(OFFSET)max_lookback )  &&  ( position<(OFFSET)(actual_len-2) )  ) // if we can proceed at all
	{
		was_match = 1; // there was match at codelen-1

		for( codelen=3; ( codelen<=max_length )&&( position<(OFFSET)(actual_len-codelen+1) ); /*nothing*/ )
		{
			if( was_match ) // for codelen-1
			{
				// codelen-1 bytes are matched, compare one more byte
				if( wrk.indata[position+codelen-1] == wrk.indata[curr_tb->pos+codelen-1] )
				{
					// add code to the table
					codes[codepos].length = codelen;
					codes[codepos].disp   = -(LONG)(position - curr_tb->pos);
					codepos++;

					codelen++; // next time do comparision of greater size
				}
				else // last bytes do not match
				{

MATCH_FAIL: // entrance for failed matches here: used 3-fold so we set "goto" here

					// go for older twobyter
					curr_tb = curr_tb->next;

					// no more twobyters or they are too far - stop search at all
					if( !curr_tb ) break;
					if( (position - curr_tb->pos)>(OFFSET)max_lookback ) break;

					// mark there was no matches
					was_match = 0;
				}
			}
			else // there were no matches for previous codelen
			{
				// next twobyter is already taken, but no comparision is done for codelen bytes
				// first we check if we need to do such comparision at all by seeing to the hashes of the ends of strings
				if( hash[position+codelen-1] == hash[curr_tb->pos+codelen-1] )
				{	// hashes match, so try matching complete string
					if( !memcmp( &wrk.indata[position], &wrk.indata[curr_tb->pos], codelen ) )
					{
						was_match = 1;
						codes[codepos].length = codelen;
						codes[codepos].disp   = -(LONG)(position - curr_tb->pos);
						codepos++;

						codelen++;
					}
					else
						// no match of whole string
						goto MATCH_FAIL;
				}
				else
					// no match of hashes
					goto MATCH_FAIL;
			}
		}
	}

	// here we assume to have found all possible matches. check for codes[] table overflow:
	// there could be matches for length 1..3839, and there is copy-1-byte, 16 copymanybyters, 1 insertion match, total 3857 entries for hrust, 256 for megalz & hrum
	// 65536 for zx7
	if(   codepos > ( (wrk.packtype==PK_HST) ? 3857 : (wrk.packtype==PK_ZX7) ? 65536 : 256 )   ) // this should not happen!
	{
		printf("mhmt-lz.c:make_lz_codes() encountered too many entries in codes[] table. Fatal error.\n");
		exit(1);
	}

	// mark end-of-records in codes[]
	codes[codepos].length = 0;
	codes[codepos].disp   = 0;
}







// returns price in bits or zero if error
//										
ULONG get_lz_price_megalz(OFFSET position, struct lzcode * lzcode)
{
	ULONG varbits,varlen;
	LONG length,disp;

	length = lzcode->length;
	disp   = lzcode->disp;

	if( length==1 )
	{
		if( disp==0 )
			return 9;
		else if( (-8)<=disp && disp<=(-1) )
			return 6;
		else
			goto INVALID_CODE_MEGALZ;
	}
	else if( length==2 )
	{
		if( (-256)<=disp && disp<=(-1) )
			return 11;
		else
			goto INVALID_CODE_MEGALZ;
	}
	else if( length==3 )
	{
		if( (-256)<=disp && disp<=(-1) )
			return 12;
		else if( (-4352)<=disp && disp<(-256) )
			return 16;
		else
			goto INVALID_CODE_MEGALZ;
	}
	else if( 4<=length && length<=255 )
	{
		varlen = 0;
		varbits = (length-2)>>1;
		while( varbits )
		{
			varbits >>= 1;
			varlen+=2;
		}

		if( (-256)<=disp && disp<=(-1) )
			varlen += 9;
		else if( (-4352)<=disp && disp<(-256) )
			varlen += 13;
		else
			goto INVALID_CODE_MEGALZ;

		return varlen+3;
	}
	else
	{
INVALID_CODE_MEGALZ:
		printf("mhmt-lz.c:get_lz_price_megalz(): Found invalid code length=%d, displacement=%d\n",length, disp);
		return 0;
	}
}


ULONG get_lz_price_hrum(OFFSET position, struct lzcode * lzcode)
{
	ULONG varlen;
	LONG length,disp;

	length = lzcode->length;
	disp   = lzcode->disp;

	if( length==1 )
	{
		if( disp==0 )
			return 9;
		else if( (-8)<=disp && disp<=(-1) )
			return 6;
		else
			goto INVALID_CODE_HRUM;
	}
	else if( length==2 )
	{
		if( (-256)<=disp && disp<=(-1) )
			return 11;
		else
			goto INVALID_CODE_HRUM;
	}
	else if (3<=length && length<=255)
	{
		varlen = 3;

		if( 4<=length && length<=15 )
		{
			varlen = 5;
			if( length>=6 ) varlen += 2;
			if( length>=9 ) varlen += 2;
			if( length>=12) varlen += 2;
		}
		else if( 15<length && length<=255 )
		{
			varlen = 13;
		}

		if( (-256)<=disp && disp<=(-1) )
			varlen += 9;
		else if( (-4096)<=disp && disp<(-256) )
			varlen += 13;
		else
			goto INVALID_CODE_HRUM;

		return varlen;
	}
	else
	{
INVALID_CODE_HRUM:
		printf("mhmt-lz.c:get_lz_price_hrum(): Found invalid code length=%d, displacement=%d\n",length, disp);
		return 0;
	}
}











ULONG get_lz_price_hrust(OFFSET position, struct lzcode * lzcode)
{
	ULONG /*varbits,*/varlen;
	LONG length,disp,tmp;

	length = lzcode->length;
	disp   = lzcode->disp;


	if( disp==0 )
	{
		if( length==1 )
		{
			return 9; // copy-1-byte
		}
		else if( (12<=length) && (length<=42) && ( !(length&1) ) )
		{
			return 11 + 8*length;
		}
		else
			goto INVALID_CODE_HRUST;
	}
	else if( length==(-3) ) // insertion match!
	{
		if( (-16)<=disp && disp<=(-1) )
		{
			return 10+8;
		}
		else if( (-79)<=disp && disp<(-16) )
		{
			return 5+8+8;
		}
		else
			goto INVALID_CODE_HRUST;
	}
	else if( length==1 )
	{
		if( (-8)<=disp && disp<=(-1) )
			return 6;
		else
			goto INVALID_CODE_HRUST;
	}
	else if( length==2 )
	{
		if( (-32)<=disp && disp<=(-1) )
		{
			return 10;
		}
		else if( (-768)<=disp && disp<(-32) )
		{
			return 13;
		}
		else
			goto INVALID_CODE_HRUST;
	}
	else if (3<=length && length<=3839 && (-65536)<=disp && disp<=(-1) )
	{
		// first, calc influence of length
		if( length<=15 ) // 3..15
		{
			varlen = 3 + ( (length/3)<<1 );

			if( length==3 )  varlen = 3;

			if( length==15 ) varlen = 11;
		}
		else if( length<=127 ) // 16..127
		{
			varlen = 14;
		}
		else // 128..3839
		{
			varlen = 14+8;
		}


		// add displacement length
		if( (-32)<=disp ) // ffe0..ffff
		{
			varlen += 7;
		}
		else if( (-512)<=disp ) // fe00..ffdf
		{
			varlen += 10;
		}
		else // 0000(-65536)..fdff: -513:-1024, -1025:-2048, -2049:-4096, ... ,-32769:-65536
		{    // bits:                   12           13           14               18

			varlen += 12;

			if( (position-wrk.prelen)>32768LL )
			{
				varlen += 6; // 8bits
			}
			else
			{
				tmp = 1024;

				while( (OFFSET)(position-wrk.prelen)>(OFFSET)tmp )
				{
					varlen++;

					tmp <<= 1;
				}
			}
		}

		return varlen;
	}
	else
	{
INVALID_CODE_HRUST:
		printf("mhmt-lz.c:get_lz_price_hrust(): Found invalid code length=%d, displacement=%d\n",length, disp);
		return 0;
	}
}




ULONG get_lz_price_zx7(OFFSET position, struct lzcode * lzcode)
{
	ULONG varbits,varlen;
	LONG length,disp;

	length = lzcode->length;
	disp   = lzcode->disp;

	if( length==1 )
	{
		if( disp==0 )
			return 9;
		else
			goto INVALID_CODE_ZX7;
	}
	else if( 2<=length && length<=65536 )
	{
		varbits = length-1;
		varlen=0;
		while( varbits )
		{
			varbits>>=1;
			varlen+=2;
		}

		if( (-128)<=disp && disp<=(-1) )
			varlen += 8;
		else if( (-2176)<=disp && disp<(-128) )
			varlen += 12;
		else
			goto INVALID_CODE_ZX7;

		return varlen;
	}
	else
	{
INVALID_CODE_ZX7:
		printf("mhmt-lz.c:get_lz_price_zx7(): Found invalid code length=%d, displacement=%d\n",length, disp);
		return 0;
	}
}
