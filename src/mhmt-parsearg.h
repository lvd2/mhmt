#ifndef MHMT_PARSEARG_H

#define MHMT_PARSEARG_H


#include "mhmt-types.h"



// string identifiers which are to be repeated somewhere
#define ARGSTR_MEGALZ "mlz"
#define ARGSTR_HRUM   "hrm"
#define ARGSTR_HRUST  "hst"
#define ARGSTR_ZX7    "zx7"

#define ARGSTR_8  "8"
#define ARGSTR_16 "16"

#define ARGSTR_MW256   "maxwin256"
#define ARGSTR_MW512   "maxwin512"
#define ARGSTR_MW1024  "maxwin1024"
#define ARGSTR_MW2048  "maxwin2048"
#define ARGSTR_MW2176  "maxwin2176"
#define ARGSTR_MW4096  "maxwin4096"
#define ARGSTR_MW4352  "maxwin4352"
#define ARGSTR_MW8192  "maxwin8192"
#define ARGSTR_MW16384 "maxwin16384"
#define ARGSTR_MW32768 "maxwin32768"
#define ARGSTR_MW65536 "maxwin65536"

#define ARGSTR_PB "prebin"

//argument types
#define ARG_INIT    0 // not to be placed in srgtbl.type!
#define ARG_MODE    1 // pack/depack
#define ARG_GREEDY  2
#define ARG_PTYPE   3 // pack type (megalz/hrum/hrust)
#define ARG_ZXHEAD  4
#define ARG_WORD    5
#define ARG_BIGEND  6
#define ARG_MAXWIN  7
#define ARG_PREBIN  8 // name of prebinary
#define ARG_NOARG 255 // just top-fill value for argstore[]


// argument table to match commandline args
struct argtbl
{
	char * name;
	ULONG type;
	char * fname;
};



// size of temporary argument storing array in parse_args()
#define ARG_STORE_SIZE 17

// return bit values for parse_args()
// possible combinations:
// SHOWHELP with or without ERROR,
// GO without ERROR,
// ERROR alone,
// nothing (if no args at all)
#define ARG_PARSER_SHOWHELP 1
#define ARG_PARSER_GO       2
#define ARG_PARSER_ERROR    256

ULONG parse_args(int argc, char* argv[]);

void sort_args( struct argtbl * args, ULONG argsize );

LONG get_maxwin( char * txtmaxwin );

struct argtbl * match_arg(char * argument);

LONG cmp_str_nocase(char * left, char * right);






#endif

