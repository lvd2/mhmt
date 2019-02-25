// this is not a code to be separately compiled!
// instead, it is included in mhmt-depack.c several times and depends on existing at include moment #define's to generate some compiling code
//
//                   // example defines:
//#define DPK_CHECK  // check input stream for consistency
//#define DPK_DEPACK // do depacking
//#define DPK_REPERR // report errors via printf
{
	ULONG check;
	ULONG byte,bits/*,bitlen*/;
	LONG disp;
	ULONG length;
	ULONG len_bits;

	ULONG stop;


	ULONG success = 1;





	// rewind input stream
	//
	check = depack_getbyte(DEPACK_GETBYTE_REWIND);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
 #ifdef DPK_REPERR
		printf("mhmt-depack-zx7.c:{} - Can't rewind input stream!\n");
 #endif
		return 0;
	}
#endif


	// first byte of input stream goes to the output unchanged
	//
	byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
//printf("FIRST: #%02x\n\n",byte);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == byte )
	{
NO_BYTE:
 #ifdef DPK_REPERR
		printf("mhmt-depack-zx7.c:{} - Can't get byte from input stream!\n");
 #endif
		return 0;
	}
#endif

#ifdef DPK_DEPACK
	success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
#endif


	// next is byte to the bitstream
	//
	check = depack_getbits(8,DEPACK_GETBITS_FORCE);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
NO_BITS:
 #ifdef DPK_REPERR
		printf("mhmt-depack-zx7.c:{} - Can't get bits from input stream!\n");
 #endif
		return 0;
	}
#endif



	// now normal depacking loop
	//
	stop = 0;
	while( (!stop) && success )
 	{
		bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
		if( 0xFFFFFFFF == bits ) goto NO_BITS;
#endif

		if( !(1&bits) ) // %0<byte>
		{
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
//printf("BITS: %%0\nBYTE: #%02x\n\n",byte);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == byte ) goto NO_BYTE;
#endif

#ifdef DPK_DEPACK
			success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
#endif
		}
		else // %100...01[len], 0 to 15 zeros, 16 zeros=exit, >16 zeros=error
		{
			len_bits = 0;
//printf("BITS: %%1");
			while( 1 )
			{
				bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
//printf("%01d",(bits&1)?1:0);
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits ) goto NO_BITS;
#endif
				if( 1 & bits )
					break;

				len_bits++;
			}


			if( len_bits >= 16 )
			{
				stop = 1;
#ifdef DPK_CHECK
				if( len_bits > 16 )
				{
					success = 0;
 #ifdef DPK_REPERR
					printf("mhmt-depack-zx7.c:{} - Too big bit length in input bitstream!\n");
 #endif
				}
#endif
				break;
			}

			// read length bits (0 to 15)
			if( len_bits > 0 )
			{
				bits = depack_getbits(len_bits,DEPACK_GETBITS_NEXT);
//int i;
//for(i=len_bits;i>0;i--){
//printf("%01d",(bits&(1<<(i-1)))?1:0);}
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits ) goto NO_BITS;
#endif
			}
			else
			{
				bits = 0;
			}

			length = 1 + (1<<len_bits) + ( bits & ((1<<len_bits)-1) );
//printf("\n");


			// now read displacement
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == byte ) goto NO_BYTE;
#endif
//printf("BYTE: #%02x\n",byte);
			disp = -(1 + byte);

			if( 128 & byte ) // bigger displacement
			{
				bits = depack_getbits(4,DEPACK_GETBITS_NEXT);
//int i;
//printf("BITS: %%");
//for(i=4;i>0;i--){
//printf("%01d",(bits&(1<<(i-1)))?1:0);}
//printf("\n");
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits ) goto NO_BITS;
#endif
				disp -= (bits&15)<<7;
			}
//printf("\n");


#ifdef DPK_CHECK
			// check disp/length
			if( (ULONG)(-disp) > wrk.maxwin )
			{
 #ifdef DPK_REPERR
				printf("mhmt-depack-zx7.c:{} - Wrong lookback displacement of %d, greater than maxwin\n",(-disp) );
 #endif
				return 0;
			}
#endif

#ifdef DPK_DEPACK
			success = success && depack_repeat(disp,length);
#endif

		}

	}

	return success;
}

