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
	LONG  disp;
	ULONG length;

	ULONG stop;


	ULONG success = 1;


	// rewind input stream
	//
	check = depack_getbyte(DEPACK_GETBYTE_REWIND);
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrum.c:{} - Can't rewind input stream!\n");
 #endif
		return 0;
	}
#endif


	// manage zx header if needed
	if( wrk.zxheader )
	{
		// skip 5 bytes (they will go to the end of output file)
		for(i=0;i<5;i++)
		{
			check = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HRM;
#endif
		}

		// next 2 bytes must be 0x10
		for(i=0;i<2;i++)
		{
			check = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HRM;
			if( check != 0x0010 )
			{
 #ifdef DPK_REPERR
				printf("mhmt-depack-hrum.c:{} - Wrong ZX-header!\n");
 #endif
				return 0;
			}
#endif
		}
	}



	// initialize bitstream first
	//
	check = depack_getbits(16,DEPACK_GETBITS_FORCE); // number 16 is ignored! - just for convenience here...
#ifdef DPK_CHECK
	if( 0xFFFFFFFF == check )
	{
NO_BITS_HRM:
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrum.c:{} - Can't get bits from input stream!\n");
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
NO_BYTE_HRM:
 #ifdef DPK_REPERR
		printf("mhmt-depack-hrum.c:{} - Can't get byte from input stream!\n");
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
		bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
		if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif

		if( 1&bits ) // %1<byte>
		{
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == byte ) goto NO_BYTE_HRM;
#endif

#ifdef DPK_DEPACK
			success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
#endif
		}
		else // %0xx
		{
			bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
			if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif

			switch( 0x03 & bits )
			{
			case 0x00: // %000xxx

				bits = depack_getbits(3,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif

				disp = (-8) | (bits&0x07); // FFFFFFF8..FFFFFFFF (-8..-1)
#ifdef DPK_CHECK
				if( (ULONG)(-disp) > wrk.maxwin )
				{
WRONG_DISP_HRM:
 #ifdef DPK_REPERR
					printf("mhmt-depack-hrum.c:{} - Wrong lookback displacement of %d, greater than maxwin\n",(-disp) );
 #endif
					return 0;
				}
#endif

#ifdef DPK_DEPACK
				success = success && depack_repeat(disp,1);
#endif
				break;


			case 0x01: // %001

				byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
				if( 0xFFFFFFFF == byte ) goto NO_BYTE_HRM;
#endif

				disp = (-256) | (0x00FF&byte); // -1..-256
#ifdef DPK_CHECK
				if( (ULONG)(-disp) > wrk.maxwin ) goto WRONG_DISP_HRM;
#endif

#ifdef DPK_DEPACK
				success = success && depack_repeat(disp,2);
#endif
				break;

			default: // %010 or %011

				if( (bits&3)==2 ) // %010 - 3 bytes
				{
					length = 3;
				}
				else // %011 - varlen
				{
					bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
					//  fetch len
					if( bits == 0x00 ) // %01100<len>, if <len>==0 - stop
					{
						length = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == length ) goto NO_BYTE_HRM;
#endif
						if( length == 0 )
							stop = 1;
					}
					else if( bits == 0x01 ) // %01101 - len=4
					{
						length = 4;
					}
					else if( bits == 0x02 ) // %01110 - len=5
					{
						length = 5;
					}
					else // %01111
					{
						bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
						if( bits == 0x00 ) // %0111100
						{
							length = 6;
						}
						else if( bits == 0x01 ) // %0111101
						{
							length = 7;
						}
						else if( bits == 0x02 ) // %0111110
						{
							length = 8;
						}
						else // %0111111
						{
							bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
							if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
							if( bits == 0x00 ) // %011111100
							{
								length = 9;
							}
							else if( bits == 0x01 ) // %011111101
							{
								length = 10;
							}
							else if( bits == 0x02 ) // %011111110
							{
								length = 11;
							}
							else // %011111111
							{
								bits = depack_getbits(2,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
								if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
								if( bits == 0x00 ) // %01111111100
								{
									length = 12;
								}
								else if( bits == 0x01 ) // %01111111101
								{
									length = 13;
								}
								else if( bits == 0x02 ) // %01111111110
								{
									length = 14;
								}
								else // %01111111111
								{
									length = 15;
								}
							}
						}
					}
				}


				// fetch disp and depack
				if( !stop )
				{
					bits = depack_getbits(1,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
					if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
					if( bits == 0x00 ) // %0<disp>
					{
						byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == byte ) goto NO_BYTE_HRM;
#endif
						disp = (-256) | (0x00FF&byte); // -1..-256
#ifdef DPK_CHECK
						if( (ULONG)(-disp) > wrk.maxwin ) goto WRONG_DISP_HRM;
#endif
#ifdef DPK_DEPACK
						success = success && depack_repeat(disp,length);
#endif
					}
					else // %1abcd<disp>
					{
						bits = depack_getbits(4,DEPACK_GETBITS_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == bits ) goto NO_BITS_HRM;
#endif
						byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
#ifdef DPK_CHECK
						if( 0xFFFFFFFF == byte ) goto NO_BYTE_HRM;
#endif
						disp = (-4096) | (0x0F00&(bits<<8)) | (0x00FF&byte); // -1..-4096
#ifdef DPK_CHECK
						if( (ULONG)(-disp) > wrk.maxwin ) goto WRONG_DISP_HRM;
#endif
#ifdef DPK_DEPACK
						success = success && depack_repeat(disp,length);
#endif
					}
				}

				break;
			}
		}
	}

	//manage zxheader again (copy to the end of output)
#ifdef DPK_DEPACK
	if( wrk.zxheader )
	{
		check = depack_getbyte(DEPACK_GETBYTE_REWIND);
 #ifdef DPK_CHECK
		if( 0xFFFFFFFF == check )
		{
  #ifdef DPK_REPERR
			printf("mhmt-depack-hrum.c:{} - Can't rewind input stream!\n");
  #endif
			return 0;
		}
 #endif

		// place 5 bytes of header to the end
		for(i=0;i<5;i++)
		{
			byte = depack_getbyte(DEPACK_GETBYTE_NEXT);
 #ifdef DPK_CHECK
			if( 0xFFFFFFFF == check ) goto NO_BYTE_HRM:
 #endif
			success = success && depack_outbyte( (UBYTE)(0x00FF&byte), DEPACK_OUTBYTE_ADD );
		}
	}
#endif

	return success;
}

