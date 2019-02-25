#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mhmt-types.h"
#include "mhmt-parsearg.h"
#include "mhmt-globals.h"


struct argtbl default_arg_table[] =
{
	{"d",            ARG_MODE,   NULL},

	{"g",            ARG_GREEDY, NULL},

	{ARGSTR_MEGALZ,  ARG_PTYPE,  NULL},
	{ARGSTR_HRUM,    ARG_PTYPE,  NULL},
	{ARGSTR_HRUST,   ARG_PTYPE,  NULL},
	{ARGSTR_ZX7,     ARG_PTYPE,  NULL},

	{"zxh",          ARG_ZXHEAD, NULL},

	{ARGSTR_8,       ARG_WORD,   NULL},
	{ARGSTR_16,      ARG_WORD,   NULL},

	{"bend",         ARG_BIGEND, NULL},

	{ARGSTR_MW256,   ARG_MAXWIN, NULL},
	{ARGSTR_MW512,   ARG_MAXWIN, NULL},
	{ARGSTR_MW1024,  ARG_MAXWIN, NULL},
	{ARGSTR_MW2048,  ARG_MAXWIN, NULL},
	{ARGSTR_MW2176,  ARG_MAXWIN, NULL},
	{ARGSTR_MW4096,  ARG_MAXWIN, NULL},
	{ARGSTR_MW4352,  ARG_MAXWIN, NULL},
	{ARGSTR_MW8192,  ARG_MAXWIN, NULL},
	{ARGSTR_MW16384, ARG_MAXWIN, NULL},
	{ARGSTR_MW32768, ARG_MAXWIN, NULL},
	{ARGSTR_MW65536, ARG_MAXWIN, NULL},

	{ARGSTR_PB,      ARG_PREBIN, NULL},

	{NULL,           0,          NULL}
};



// main argument parser, sets fields of "struct globals wrk",
// tries to detect all erroneous arguments.
ULONG parse_args(int argc, char* argv[])
{
	struct argtbl argstore[ARG_STORE_SIZE+1]; // last element is always stop-value



	struct argtbl * arg;

	ULONG inarg_pos; // position in argv[] array
	ULONG storearg_pos; // position in argstore[] array

	ULONG files_num; // number of filenames specified

	char * temp_filename;

	ULONG last_arg_type;

	ULONG maxwin;


	ULONG i;
	for(i=0;i<ARG_STORE_SIZE+1;i++)
	{
		argstore[i].name  = NULL;
		argstore[i].fname = NULL;
		argstore[i].type  = ARG_NOARG;
	}


	if( argc<2 )
	{
		printf("No arguments! Use \"mhmt -h\" or \"mhmt -help\" for help!\n");
		return 0L;
	}

	// shortcut for help request
	if( !cmp_str_nocase( argv[1]+1, "h" ) || !cmp_str_nocase( argv[1]+1, "help" ) )
		return ARG_PARSER_SHOWHELP;


	inarg_pos = 1;
	storearg_pos = 0;
	files_num = 0;

	// first find all arguments beginning with "-"
	while( inarg_pos<(ULONG)argc && *(argv[inarg_pos])=='-' )
	{
		arg=match_arg(argv[inarg_pos]+1); // search match...
		if( arg ) // match!
		{
			if( storearg_pos>=ARG_STORE_SIZE )
			{
				printf("Too many arguments!\n");
				return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
			}

			argstore[storearg_pos] = *arg;

			if( arg->type == ARG_PREBIN )
			{
				if( inarg_pos>=(ULONG)(argc-1) )
				{
					printf("\"-prebin\" has no filename!\n");
					return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
				}

				inarg_pos++;
				
				argstore[storearg_pos].fname = (char *)malloc( 1+strlen(argv[inarg_pos]) );
				if( !argstore[storearg_pos].fname )
				{
					printf("Cannot allocate memory for filename string!\n");
					return ARG_PARSER_ERROR;
				}

				strcpy( argstore[storearg_pos].fname, argv[inarg_pos] );
			}

			storearg_pos++;
		}
		else // argument does not match predefined set
		{
			printf("Wrong arguments!\n");
			return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
		}

		inarg_pos++;
	}

	// parse filenames then
	while( inarg_pos<(ULONG)argc )
	{
		if( files_num>=2 ) // there should be no more than two filenames
		{
			printf("Too many filenames specified!\n");
			return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
		}

		temp_filename = (char *)malloc( 1+strlen(argv[inarg_pos]) );
		if( !temp_filename )
		{
			printf("Cannot allocate memory for filename string!\n");
			return ARG_PARSER_ERROR;
		}

		strcpy( temp_filename, argv[inarg_pos] );

		if( files_num==0 )
			wrk.fname_in = temp_filename;
		else // only files_num==1, because of condition in the beginning of current "while" cycle
			wrk.fname_out = temp_filename;

		files_num++;
		inarg_pos++;
	}

	if( !files_num ) // there must be at least 1 filename specified
	{
		printf("No filenames specified!\n");
		return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
	}


	// now optional arguments (starting with "-") are stored in argstore[],
	// all needed filenames are also copied, go proceed configuring with
	// optional arguments

	// sort argument array (in increasing .type order) to ensure correct parsing
	sort_args( argstore, ARG_STORE_SIZE );

	storearg_pos = 0;
	last_arg_type = ARG_INIT; // there is no such value in argstore[].type
	while( argstore[storearg_pos].type != ARG_NOARG )
	{
		if( last_arg_type == argstore[storearg_pos].type )
		{
			printf("Redundant arguments!\n");
			return ARG_PARSER_ERROR|ARG_PARSER_SHOWHELP;
		}

		switch( argstore[storearg_pos].type )
		{
		case ARG_MODE:
			wrk.mode = 1; // set depack mode
			break;

		case ARG_GREEDY:
			if( wrk.mode ) // since sorted, argument list causes parsing go from up to down in this "case" list
			{
				printf("No greedy mode specification for DEpacking!\n");
				return ARG_PARSER_ERROR;
			}
			wrk.greedy = 1; // set greedy packing mode
			break;

		case ARG_PTYPE:
			if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_MEGALZ ) )
			{
				wrk.packtype = PK_MLZ;
				wrk.zxheader = 0;
				wrk.wordbit  = 0;
				wrk.bigend   = 0;
				wrk.fullbits = 0;
				wrk.maxwin   = 4352;
			}
			else if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_HRUM ) )
			{
				wrk.packtype = PK_HRM;
				wrk.zxheader = 0; // by default, there is NO ZX-HEADER if only -hrm or -hst specified
				wrk.wordbit  = 1;
				wrk.bigend   = 0;
				wrk.fullbits = 1;
				wrk.maxwin   = 4096;
			}
			else if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_HRUST ) )
			{
				wrk.packtype = PK_HST;
				wrk.zxheader = 0; // by default, there is NO ZX-HEADER if only -hrm or -hst specified
				wrk.wordbit  = 1;
				wrk.bigend   = 0;
				wrk.fullbits = 1;
				wrk.maxwin   = 65536;
			}
			else if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_ZX7 ) )
			{
				wrk.packtype = PK_ZX7;
				wrk.zxheader = 0;
				wrk.wordbit  = 0;
				wrk.bigend   = 0;
				wrk.fullbits = 0;
				wrk.maxwin   = 2176;
			}
			else // there shouldn't be this case, but nevertheless...
			{
				printf("Impossible error #1! Press any key to continue or \"SPACE\" to exit... :-)\n");
				return ARG_PARSER_ERROR;
			}
			break;

		case ARG_ZXHEAD:
			// ZX-header is not applicable for PK_MLZ type...
			// also wrk.packtype has been already set before...
			if( PK_MLZ==wrk.packtype )
			{
				printf("There couldn't be zx-header in megalz mode!\n");
				return ARG_PARSER_ERROR;
			}
			wrk.zxheader = 1;
			break;

		case ARG_WORD:
			// whether bits must be grouped in words or in bytes
			if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_8 ) )
			{
				if( wrk.zxheader ) // won't force byte-wise bits when there is a zx-header
				{
					printf("There can be only 16bit grouping of bits when ZX-header is active!\n");
					return ARG_PARSER_ERROR;
				}
				wrk.wordbit = 0;
			}
			else if( !cmp_str_nocase( argstore[storearg_pos].name, ARGSTR_16 ) )
			{
				wrk.wordbit = 1;
			}
			else // there shouldn't be this case, but nevertheless...
			{
				printf("Impossible error #2! Press any key to continue or \"SPACE\" to exit... :-)\n");
				return ARG_PARSER_ERROR;
			}
			break;

		case ARG_BIGEND:
			// whether word-grouped bits must be big- or little-endian arranged
			if( wrk.zxheader )
			{
				printf("There can be only little-endian arrangement of bits when ZX-header is active!\n");
				return ARG_PARSER_ERROR;
			}
			wrk.bigend = 1;
			break;

		case ARG_MAXWIN:
			maxwin = get_maxwin( argstore[storearg_pos].name );
			if( !maxwin ) // there shouldn't be this case, but nevertheless...
			{
				printf("Impossible error #3! Press any key to continue or \"SPACE\" to exit... :-)\n");
				return ARG_PARSER_ERROR;
			}

                        // wrk.maxwin is already initialized to the maximum value suitable for given packing type, so check new setting
			if( maxwin > wrk.maxwin )
			{
				printf("Maximum window specified is too big for given packing type!\n");
				return ARG_PARSER_ERROR;
			}
			wrk.maxwin = maxwin;
			break;

		case ARG_PREBIN:
			wrk.prebin = 1;
			wrk.fname_prebin = argstore[storearg_pos].fname;
			break;

		default:
			// once again impossible error: we shouldn't be here since "while" loop condition...
			printf("Impossible error #4! Press any key to continue or \"SPACE\" to exit... :-)\n");
			return ARG_PARSER_ERROR;
			break;
		}


		last_arg_type = argstore[storearg_pos++].type;
	}


	return ARG_PARSER_GO;
}


// sort arguments
void sort_args( struct argtbl * args, ULONG argsize )
{
	struct argtbl temp;
	LONG i,j;

	// simple bubble sort since there are not too many arguments
	for( i=(argsize-2); i>=0; i-- )
	{
		for( j=0; j<=i; j++ )
		{
			if( args[j].type > args[j+1].type )
			{
				temp      = args[j];
				args[j]   = args[j+1];
				args[j+1] = temp;
			}
		}
	}
}


// get maxwin string into number, returns 0 if no match
LONG get_maxwin( char * txtmaxwin )
{
	static char * strings[] =
	{
		ARGSTR_MW256,
		ARGSTR_MW512,
		ARGSTR_MW1024,
		ARGSTR_MW2048,
		ARGSTR_MW4096,
		ARGSTR_MW4352,
		ARGSTR_MW8192,
		ARGSTR_MW16384,
		ARGSTR_MW32768,
		ARGSTR_MW65536,
		NULL
	};

	static LONG sizes[] =
	{
		256,
		512,
		1024,
		2048,
		4096,
		4352,
		8192,
		16384,
		32768,
		65536,
		0
	};

	ULONG i;


	i=0;
	while( strings[i] )
	{
		if( !cmp_str_nocase( strings[i], txtmaxwin ) )
		{
			return sizes[i];
		}

		i++;
	}

	return 0;
}





// finds matching arg in default_arg_table,
// returns ptr to the found element or NULL if not found
struct argtbl * match_arg(char * argument)
{
	struct argtbl * test_arg = default_arg_table;


	while( test_arg->name && cmp_str_nocase(test_arg->name,argument) )
		test_arg++;

	return (test_arg->name)?test_arg:NULL;
}


// compares two char strings, ignoring case (uppercase by default)
// returns 0, if equal, -1 if left lower than right, and +1 otherwise
LONG cmp_str_nocase(char * left, char * right)
{

	LONG order=0;

	UBYTE leftchar,rightchar;
	UBYTE leftadd,rightadd;

	do
	{
		leftchar  = (UBYTE)*left;
		rightchar = (UBYTE)*right;

		leftadd  = 0;
		rightadd = 0;

		left++;
		right++;

		if( leftchar  >= (UBYTE)'a' ) leftadd  = (UBYTE)('A'-'a');
		if( rightchar >= (UBYTE)'a' ) rightadd = (UBYTE)('A'-'a');

		if( leftchar  > (UBYTE)'z' ) leftadd  = 0;
		if( rightchar > (UBYTE)'z' ) rightadd = 0;

		leftchar  += leftadd;
		rightchar += rightadd;

		if( leftchar<rightchar ) order = (-1);
		if( leftchar>rightchar ) order = (+1);

	} while( (!order) && leftchar && rightchar );

	return order;
}

