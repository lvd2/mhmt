// this is not a code to be separately compiled!
// instead, it is included in mhmt-depack.c several times and depends on existing at include moment #define's to generate some compiling code
//
//                   // example defines:
//#define DPK_CHECK  // check input stream for consistency
//#define DPK_DEPACK // do depacking
//#define DPK_REPERR // report errors via printf
{
	LONG i;

	ULONG check;
	ULONG byte,bits;//,bitlen;

	LONG   disp=0;
	LONG length=0; // if -3 - insertion match, if 0 - nothing to do

	ULONG skipdisp,skiplen;

	ULONG disptype;
// fetch byte and add to existing disp
#define DISP_PLUSBYTE 0
// fetch 5 bits to disp
#define DISP_ABCDE    1
// common disp for 3+ lengthes
#define DISP_COMMON   2

	ULONG docopy; // do copy from input instead of repeating

	ULONG expbitlen = 2; // expandable displacement


	ULONG stop;


	ULONG success = 1;





	// rewind input stream
	//
	check = depack_getbyte(DEPACK_GETBYTE_REWIND);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrust.c:{} - Can't rewind input stream!\n");
 #endif
		return 0;
	}
#endif


	// manage zx header if needed
	if( wrk.zxheader )
	{
		// check for "HR" in beginning
		check = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
		if( 0xFFFFFFFF == check ) goto NO_BYTE_HST;
		if( check != 'H' )
		{
 #ifdef DPK_REPERR
			printf("mhmt-depack-hrust.c:{} - Bad zx-header!\n");
 #endif
			return 0;
		}
#endif
		check = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
		if( 0xFFFFFFFF == check ) goto NO_BYTE_HST;
		if( check != 'R' )
		{
 #ifdef DPK_REPERR
			printf("mhmt-depack-hrust.c:{} - Bad zx-header!\n");
 #endif
			return 0;
		}
#endif

		// skip 10 bytes
		for(i=0;i<10;i++)
		{
			check = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HST;
#endif
		}
	}



	// initialize bitstream first
	//
	check = depack_getbits(16,DEPACK_GETBITS_FORCE); // number 16 is ignored! - just for convenience here...
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
NO_BITS_HST:
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrust.c:{} - Can't get bits from input stream!\n");
 #endif
		return 0;
	}
#endif



	// then byte of input stream goes to the output unchanged
	//
	byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == byte )
	{
NO_BYTE_HST:
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrust.c:{} - Can't get byte from input stream!\n");
 #endif
		return 0;
	}
#endif

#ifdef DPK_DEPACK
	success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
#endif



	// now normal depacking loop
	//
  	stop = 0;
	while( (!stop) && success )
 	{
		skiplen  = 0;
		skipdisp = 0;
		disptype = DISP_PLUSBYTE;
		docopy = 0;

		bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
		if( 0xFFFFFFFF == bits )
		{
			#ifdef DBG
			printf("line %d\n",__LINE__);
			#endif
			goto NO_BITS_HST;
		}
#endif

		if( 1&bits ) // %1<byte>
		{
			docopy = 1;
			length = 1;
		}
		else // %0xx
		{
			bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == bits )
			{
				#ifdef DBG
				printf("line %d\n",__LINE__);
				#endif
				goto NO_BITS_HST;
			}
#endif

			switch( bits&3 )
			{
			case 0: // %000xxx

				bits = depack_getbits(3,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits )
				{
					#ifdef DBG
					printf("line %d\n",__LINE__);
					#endif
					goto NO_BITS_HST;
				}
#endif

				disp = (-8) | (bits&0x07); // FFFFFFF8..FFFFFFFF (-8..-1)
				length = 1;

				skiplen  = 1;
				skipdisp = 1;

				break;






			case 1: // %001 - 2 bytes or insertion match part 1

				length = 2;
				skiplen = 1;

				bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits )
					{
						#ifdef DBG
						printf("line %d\n",__LINE__);
						#endif
						goto NO_BITS_HST;
					}
#endif

				switch( bits&3 )
				{
				case 0: // %001 00 - disp FDxx
					//disptype = DISP_PLUSBYTE; // default value
					disp = (-768);

					break;

				case 1: // %001 01 - disp FExx
					//disptype = DISP_PLUSBYTE; // default value
					disp = (-512);

					break;

				case 2: // %001 10 - ff00..ffdf or insertion match

					byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == byte ) goto NO_BYTE_HST;
#endif
					skipdisp = 1;

					if( byte<0x00E0 ) // ff00..ffdf
					{
						disp = (-256) | (byte&0x00FF);
					}
					else if( byte==0x00FE ) // expand bitlen of expandable displacement
					{
						#ifdef DBG
							printf("expansion\n");
						#endif

						length = 0; // nothing to do
						expbitlen++;
//#ifdef DPK_CHECK
//						if( expbitlen>8 )
//						{
// #ifdef DPK_REPERR
//							printf("mhmt-depack-hrust.c:{} - bitlen of expandable displacement expanded more than 16 bits!\n");
// #endif
//							return 0;
//						}
//#endif
						// this is the fix to mimic Z80 depacker (what seems to be perfectly correct!)
						if( expbitlen > 8 )
							expbitlen = 1;
					}
					else // insertion match - xor 2
					{
						length = (-3); // mark insertion match

						byte = ((byte<<1)&0x00FE) | ((byte>>7)&0x01); // byte<<<1
						byte ^= 0x02;
						byte -= 15;

						disp = (-256) | (byte&0x00FF);
					}
					break;

				case 3: // %001 11 - disp FFE0+[abcde]

					disptype = DISP_ABCDE;

					break;
				}

				break;



			case 2: // %010 - 3 bytes or something

				length = 3;
				skiplen = 1;
				disptype = DISP_COMMON;

				break;


			case 3: // %011 - varlen

				disptype = DISP_COMMON;

				break;
			}
		}

			if( (!stop) && (!skiplen) && (!docopy) )
			{
				// read variable length
				bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits )
				{
					#ifdef DBG
					printf("line %d\n",__LINE__);
					#endif
					goto NO_BITS_HST;
				}
#endif                                                                                 
				switch( bits&3 )
				{
				case 0: // special cases

					bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits )
					{
						#ifdef DBG
						printf("line %d\n",__LINE__);
						#endif
						goto NO_BITS_HST;
					}
#endif
					if( bits&1 ) // %011 001abcd<byte> - insertion match, displacements -1..-16
					{
						bits = depack_getbits(4,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits )
						{
							#ifdef DBG
							printf("line %d\n",__LINE__);
							#endif
							goto NO_BITS_HST;
						}
#endif
						length = (-3); // mark insertion match

						skipdisp = 1; // prepare displacement
						disp = (-16) | (bits&15);
					}
					else
					{
						bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits )
						{
							#ifdef DBG
							printf("line %d\n",__LINE__);
							#endif
							goto NO_BITS_HST;
						}
#endif
						if( bits&1 ) // %011 0001abcd - copy-many-bytes
						{
							bits = depack_getbits(4,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
							if( 0xFFFFFFFF == bits )
							{
								#ifdef DBG
								printf("line %d\n",__LINE__);
								#endif
								goto NO_BITS_HST;
							}
#endif
							length = ((bits&15)+6)<<1;
							skipdisp = 1;
							docopy = 1;
						}
						else // %011 0000abcdefg[<byte>] - longer lengthes
						{
							bits = depack_getbits(7,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
							if( 0xFFFFFFFF == bits )
							{
								#ifdef DBG
								printf("line %d\n",__LINE__);
								#endif
								goto NO_BITS_HST;
							}
#endif
							bits &= 127;

							if( bits==15 ) // stop depack
							{
								stop=1;
							}
							else if( bits>15 ) // 16..127
							{
								length = bits;
							}
							else // 0..14: longer lengthes
							{
								byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
								if( 0xFFFFFFFF == byte ) goto NO_BYTE_HST;
#endif
								length = (bits<<8) + (byte&0x00FF);
							}
						}
					}
					break;
				case 1: // %01101
					length = 4;
					break;
				case 2: // %01110
					length = 5;
					break;
				case 3: // variable length (6-15), %01111...

					length = 6;
					do
					{
						bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits )
						{
							#ifdef DBG
							printf("line %d\n",__LINE__);
							#endif
							goto NO_BITS_HST;
						}
#endif                                                                                  
						bits &= 3;
						length += bits;

					} while( (bits==3) && (length<15) );
					break;
				}
			}

			if( (!stop) && (!skipdisp) && (!docopy) )
			{
				// extract displacement

				switch( disptype )
				{
				case DISP_COMMON:

					bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits )
					{
						#ifdef DBG
						printf("line %d\n",__LINE__);
						#endif
						goto NO_BITS_HST;
					}
#endif                                                                                  
					bits &= 3;
					if( !bits ) // %00<byte> - fe00..feff
					{
						disp = (-512);
						disptype=DISP_PLUSBYTE; // we fall in next section and there is check
						// NO break!
					}
					else if( bits==1 ) // %01<byte> - ff00..ffdf
					{
						disp = (-256);
						// NO break!
						// no check for byte in range e0..ff here - but in next switch section!
					}
					else if( bits==2 ) // %10abcde - ffe0..ffff
					{
						bits = depack_getbits(5,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits )
						{
							#ifdef DBG
							printf("line %d\n",__LINE__);
							#endif
							goto NO_BITS_HST;
						}
#endif                                                                                  
						disp = (-32) | (bits&31);

						break;
					}
					else // %11... - expanding displacement
					{
						bits = depack_getbits(expbitlen,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits )
						{
							#ifdef DBG
							printf("line %d\n",__LINE__);
							#endif
							goto NO_BITS_HST;
						}
#endif                                                                                  
						disp = (-1)<<expbitlen;
						disp |= (bits&(~disp));

						disp <<= 8;

						disptype = DISP_PLUSBYTE;
						// NO break!
					}

				case DISP_PLUSBYTE:
					byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == byte ) goto NO_BYTE_HST;
#endif
					if( disptype==DISP_COMMON ) // if we here from previous section of switch()
					{                           // we must check for insertion match!

						if( byte<0x00E0 ) // ff00..ffdf
						{
							disp = (-256) | (byte&0x00FF);
						}
						else // insertion match - xor 3
						{
							length = (-3); // mark insertion match

							byte = ((byte<<1)&0x00FE) | ((byte>>7)&0x01); // byte<<<1
							byte ^= 0x03;
							byte -= 15;

							disp = (-256) | (byte&0x00FF);
						}
					}
					else
					{
 						disp = disp + (byte&0x00FF);
					}

					break;

				case DISP_ABCDE:
					bits = depack_getbits(5,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits )
					{
						#ifdef DBG
						printf("line %d\n",__LINE__);
						#endif
						goto NO_BITS_HST;
					}
#endif
					disp = (-32) | (bits&31);

					break;

				default:
#ifdef DPK_CHECK
 #ifdef DPK_REPERR
					printf("mhmt-depack-hrust.c:{} - Wrong displacement in disptype!\n");
 #endif
					return 0;
#endif
					break;
				}
			}


#ifdef DPK_CHECK
			if( success && (!docopy) && (!stop) && ((ULONG)(-disp)>wrk.maxwin) )
			{
WRONG_DISP_HST:
 #ifdef DPK_REPERR
				printf("mhmt-depack-hrust.c:{} - Wrong lookback displacement of %d, greater than maxwin\n",(-disp) );
 #endif
				return 0;
			}
#endif


			if( docopy && (!stop) )
			{
				#ifdef DBG
					printf("copy.len=%d\n",length);
				#endif
				for(i=0;i<length;i++)
				{
					byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == byte ) goto NO_BYTE_HST;
#endif


#ifdef DPK_DEPACK
					success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
#endif
				}
			}
			else if( (!docopy) && (!stop) )// not do-copy
			{
				if( length!=(-3) )
				{
					#ifdef DBG
					if(length) printf("match.len=%d,disp=%d\n",length,disp);
					#endif

#ifdef DPK_DEPACK
					if( length )
						success = success && depack_repeat(disp,length);
#endif
				}
				else // (-3)
				{
					byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == byte ) goto NO_BYTE_HST;
#endif
					#ifdef DBG
						printf("insert-match.len=%d,disp=%d\n",(-length),disp);
					#endif


#ifdef DPK_DEPACK
					success = success && depack_repeat(disp,1);
					success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
					success = success && depack_repeat(disp,1);
#endif
				}

			}
	}


	#ifdef DBG
	printf("got stop symbol!\n");
	#endif


	//manage zxheader again (copy to the end of output)
#ifdef DPK_DEPACK
	if( wrk.zxheader )
	{
		check = depack_getbyte(DEPACK_GETBYTE_REWIND);
 #ifdef DPK_CHECK
		if( 0xFFFFFFFF == check )
		{
  #ifdef DPK_REPERR
			printf("mhmt-depack-hrust.c:{} - Can't rewind input stream!\n");
  #endif
			return 0;
		}
 #endif

		for(i=0;i<6;i++) // skip bytes
		{
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
 #ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HST;
 #endif
		}
		// place 6 bytes of header to the end
		for(i=0;i<6;i++)
		{
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
 #ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HST;
 #endif
			success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
		}
	}
#endif

	return success;
}

