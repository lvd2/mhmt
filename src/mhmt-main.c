#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "mhmt-types.h"
#include "mhmt-globals.h"
#include "mhmt-parsearg.h"
#include "mhmt-pack.h"
#include "mhmt-depack.h"


void show_help(void);
void dump_config(void);
ULONG do_files(void);

int main( int argc, char* argv[] )
{
	int error=0;
	ULONG parse_result;


	init_globals();


	// printf short info
	printf("mhmt - MeHruMsT - MEgalz, HRUM and hruST (c) 2009-2016 lvd^nedopc\n\n");

	// parse arguments
	parse_result = parse_args(argc, argv);

	if( parse_result&ARG_PARSER_SHOWHELP )
	{
		if( parse_result&ARG_PARSER_ERROR )
			printf("\n");

		show_help();
	}

	if( parse_result&ARG_PARSER_ERROR )
	{
		printf("There were errors in arguments.\n");
		error++;
	}
	else if( parse_result&ARG_PARSER_GO )
	{
		if( do_files() )
		{
			dump_config();
			if( wrk.mode )
			{
				error += depack() ? 0 : 1;
			}
			else
			{
				error += pack() ? 0 : 1;
			}
		}
		else
		{
			error++;
		}
	}



	free_globals();

	return error;
}

void show_help(void)
{
	printf
	(
		"======== mhmt help ========\n"
		"parameters:\n"
		"-mlz, -hrm, -hst, -zx7 - use MegaLZ, hrum3.5, hrust1.x or zx7 formats (default is MegaLZ)\n"
		"-g - greedy coding (default is optimal coding), not supported yet\n"
		"-d - depacking instead of packing (default is packing)\n"
		"\n"
		"-zxh - use zx-specific header for hrum or hrust. DEFAULT is NO HEADER!\n"
		"       Not applicable for MegaLZ. If -zxh is specified, -16, NO -bend and\n"
		"       NO -mlz is forced.\n"
		"\n"
		"-8, -16 - bitstream is in bytes or words in packed file.\n"
		"          Default for MegaLZ is -8, for hrum and hrust is -16.\n"
		"\n"
		"-bend - if -16 specified, this makes words big-endian. Default is little-endian.\n"
		"\n"
		"-maxwinN - maximum lookback window. N is decimal number, which can only be\n"
		"           256,512,1024,2048,4096,8192,16384,32768. Default is format-specific\n"
		"           maximum window: MegaLZ is 4352, hrum is 4096, hrust is 65536,\n"
		"                           zx7 is 2176.\n"
		"           For given format, window can't be greater than default value\n"
		"\n"
		"-prebin <filename> - use specified file as prebinary for packing and depacking.\n"
		"\n"
		"usage:\n"
		"mhmt [parameter list] <input filename> [<output filename>]\n"
		"\n"
		"if no output filename given, filename is appended with \".mlz\", \".hrm\", \".hst\" or \".zx7\"\n"
		"in accordance with the format chosen; for depacking \".dpk\" is appended\n"
		"====== mhmt help end ======\n"
		"\n"
	);
}

void dump_config(void)
{
	printf("Configuration review:\n");
	printf("\n");

	printf("Pack format: ");
	if( wrk.packtype==PK_MLZ )
		printf("MegaLZ.\n");
	else if( wrk.packtype==PK_HRM )
		printf("Hrum3.5\n");
	else if( wrk.packtype==PK_HST )
		printf("Hrust1.x\n");
	else if( wrk.packtype==PK_ZX7 )
		printf("zx7\n");
	else
		printf("unknown.\n"); // this should be actually never displayed

	printf("Mode:        ");
	if( wrk.mode )
		printf("depacking.\n");
	else
		printf("packing.\n");

	if( !wrk.mode )
	{
		printf("Pack coding: ");
		if( wrk.greedy )
			printf("greedy (sub-optimal but faster).\n");
		else
			printf("optimal (slower).\n");
	}

	if( wrk.zxheader )
	{
		printf("Header for old ZX ");
		if( wrk.packtype==PK_HRM )
			printf("hrum3.5 ");
		else if( wrk.packtype==PK_HST )
			printf("hrust1.x ");

		printf("depackers is on.\n");
	}

	if( wrk.wordbit )
	{
		printf("Bitstream is grouped in words -\n");
		if( wrk.bigend )
		{
			printf(" words are big-endian, %s","INCOMPATIBLE with old ZX depackers!\n");
		}
		else
		{
			printf(" words are little-endian, ");
			if( (wrk.packtype==PK_HRM) || (wrk.packtype==PK_HST) )
				printf("compatible with old ZX depackers.\n");
			else
				printf("INCOMPATIBLE with old ZX depackers!\n");
		}
	}
	else
	{
		printf("Bitstream is grouped in bytes -\n");
		if( wrk.packtype==PK_MLZ || wrk.packtype==PK_HRM || wrk.packtype==PK_ZX7 )
			printf(" compatible with old ZX depackers.\n");
		else
			printf(" INCOMPATIBLE with old ZX depackers!\n");
	}

	printf("Maximum lookback window size is %d bytes.\n\n",wrk.maxwin);


	// files
	printf("Input file \"%s\" (%d bytes) successfully loaded.\n", wrk.fname_in, wrk.inlen);
	printf("Output file \"%s\" created.\n", wrk.fname_out );

	// prebin file
	if( wrk.prebin )
	{
		printf("Prebinary file \"%s\" (%d bytes) successfully loaded.\n", wrk.fname_prebin, wrk.prelen);
	}
	else
	{
		printf("No prebinary file specified.\n");
	}
//	...more info...?
}

// create output filename, open files, load input file in memory
// returns 1 if no errors, otherwise zero
ULONG do_files(void)
{
	char * pack_ext;
	char * depk_ext;
	LONG ext_pos;

	struct stat stfile;

	UBYTE * tmp;
	ULONG len;

	// if there is no output filename, create it
	if( !wrk.fname_out )
	{
		depk_ext = ".dpk";

		if( wrk.packtype==PK_MLZ )
			pack_ext = ".mlz";
		else if( wrk.packtype==PK_HRM )
			pack_ext = ".hrm";
		else if( wrk.packtype==PK_HST )
			pack_ext = ".hst";
		else if( wrk.packtype==PK_ZX7 )
			pack_ext = ".zx7";
		else
			pack_ext = ".pak"; // all have the same size, as well as depk_ext - 4 bytes!


		wrk.fname_out = (char *)malloc( 5 + strlen(wrk.fname_in) );
		if( !wrk.fname_out )
		{
			printf("Can't allocate memory for output filename!\n");
			return 0;
		}

		strcpy(wrk.fname_out, wrk.fname_in);

		if( !wrk.mode ) // packing
		{
			strcat(wrk.fname_out, pack_ext);
		}
		else // depacking
		{
			ext_pos = strlen( wrk.fname_out ) - 4;

			if( (ext_pos>=0) && (!strcmp(&wrk.fname_out[ext_pos], pack_ext)) )
				strcpy( &wrk.fname_out[ext_pos], depk_ext );
			else
				strcat( wrk.fname_out, depk_ext );
		}
	}


	//open files
	wrk.file_in=fopen(wrk.fname_in,"rb");
	if(!wrk.file_in)
	{
		printf("Cannot open input file \"%s\"!\n",wrk.fname_in);
		return 0;
	}

	wrk.file_out=fopen(wrk.fname_out,"wb");
	if(!wrk.file_out)
	{
		printf("Cannot create output file \"%s\"!\n",wrk.fname_out);
		return 0;
	}

	if( wrk.prebin )
	{
		wrk.file_prebin = fopen(wrk.fname_prebin,"rb");
		if(!wrk.file_prebin)
		{
			printf("Cannot open prebinary file \"%s\"!\n",wrk.fname_prebin);
			return 0;
		}
	}



	// get lengths of files
	if( fseek(wrk.file_in,0,SEEK_END) )
	{
		printf("Cannot fseek() input file \"%s\"!\n",wrk.fname_in);
		return 0;
	}
	wrk.inlen=(ULONG)ftell(wrk.file_in);
	if( wrk.inlen==(ULONG)(-1L)  )
	{
		printf("Cannot ftell() length of input file \"%s\"!\n",wrk.fname_in);
		wrk.inlen=0;
		return 0;
	}
	else if( wrk.inlen<16 )
	{
		printf("Input file \"%s\" is smaller than 16 bytes - I won't process it!\n",wrk.fname_in);
		return 0;
	}
	if( fseek(wrk.file_in,0,SEEK_SET) )
	{
		printf("Cannot fseek() input file \"%s\"!\n",wrk.fname_in);
		return 0;
	}

	if( wrk.prebin )
	{
		if( fstat( fileno(wrk.file_prebin), &stfile ) )
//		if( stat( wrk.fname_prebin, &stfile ) )
		{
			printf("Cannot fstat() prebin file \"%s\"\n",wrk.fname_prebin);
			return 0;
		}

		wrk.prelen = (ULONG)stfile.st_size;
	}



	// load files in mem
	//
	// first allocate place for both prebin and input file
	len = wrk.inlen + wrk.prelen; // wrk.prelen is 0 if wrk.prebin==0
	tmp = (UBYTE *)malloc( len );
	//
	// check alloc is OK
	if( !tmp )
	{
		if( wrk.prebin )
		{
			printf("Cannot allocate %d bytes of memory for loading both input file \"%s\" and prebin file \"%s\"!\n", len, wrk.fname_in, wrk.fname_prebin );
		}
		else
		{
			printf("Cannot allocate %d bytes of memory for loading input file \"%s\"!\n", wrk.inlen, wrk.fname_in );
		}
		return 0;
	}
	//
	// assign to wrk.indata
	wrk.indata_raw = tmp;
	wrk.indata     = tmp + (wrk.prebin ? wrk.prelen : 0); // so we access prebin using negative displacements into wrk.indata array
	//
	// load input file and prebin file
	if( wrk.inlen!=fread(wrk.indata,1,wrk.inlen,wrk.file_in) )
	{
		printf("Cannot successfully load input file \"%s\" in memory!\n",wrk.fname_in);
		return 0;
	}
	if( wrk.prebin )
	{
		if( wrk.prelen!=fread(tmp,1,wrk.prelen,wrk.file_prebin) )
		{
			printf("Cannot successfully load prebin file \"%s\" in memory!\n",wrk.fname_prebin);
			return 0;
		}
	}



	return 1;// no errors
}

