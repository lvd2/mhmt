#ifndef MHMT_DEPACK_H
#define MHMT_DEPACK_H

#include "mhmt-types.h"
#include "mhmt-lz.h"


//#define MAX_CODES_SIZE 3860 // max num of codes is 3857, plus stopcode, plus some extra bytes



ULONG depack(void);


#define DEPACK_GETBYTE_REWIND 0
#define DEPACK_GETBYTE_NEXT   1
// rewind - to the beginning of input stream, byte - next byte
// returns 0xFFFFFFFF if error, otherwise byte (0..255)
ULONG depack_getbyte(ULONG operation);

#define DEPACK_GETBITS_FORCE 1
#define DEPACK_GETBITS_NEXT  2
ULONG depack_getbits(ULONG numbits, ULONG operation);

ULONG depack_getbits_word(void);

#define DEPACK_OUTBYTE_INIT  1
#define DEPACK_OUTBYTE_FLUSH 2
#define DEPACK_OUTBYTE_ADD   3
ULONG depack_outbyte(UBYTE byte, ULONG operation);
ULONG depack_repeat(LONG disp, ULONG length);


ULONG  checker_megalz(void);
ULONG depacker_megalz(void);
ULONG  checker_hrum  (void);
ULONG depacker_hrum  (void);
ULONG  checker_hrust (void);
ULONG depacker_hrust (void);
ULONG  checker_zx7   (void);
ULONG depacker_zx7   (void);



#endif

