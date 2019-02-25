# makfile configuration
NAME            = mhmt
OBJECTS         = mhmt-emit.o mhmt-globals.o mhmt-hash.o mhmt-lz.o mhmt-main.o mhmt-optimal.o mhmt-pack.o mhmt-parsearg.o mhmt-tb.o mhmt-depack.o
SRCS            = mhmt-emit.s mhmt-globals.s mhmt-hash.s mhmt-lz.s mhmt-main.s mhmt-optimal.s mhmt-pack.s mhmt-parsearg.s mhmt-tb.s mhmt-depack.s

#CFLAGS          = -m32 -march=i386 -std=c99 -O2 -flto -fomit-frame-pointer -Wunused-variable -Wextra -Werror -pedantic-errors -g -D_POSIX_C_SOURCE=1 -U__STRICT_ANSI__ 
CFLAGS          = -march=k8 -std=c99 -O2 -flto -fomit-frame-pointer -Wunused-variable -Wextra -Werror -pedantic-errors -g -D_POSIX_C_SOURCE=1 -U__STRICT_ANSI__

#switch the compiler (for the internal make rules)
CC              = gcc
DA		= objdump -dS

#all should be the first target. it's built when make is runwithout args
all: ${NAME} ${SRCS}

strip: ${NAME}
	strip ${NAME}

#additional rules for files
${NAME}: ${OBJECTS}
	${CC} ${CFLAGS} -o $@ ${OBJECTS}

clean:
	rm -f ${NAME} ${OBJECTS} ${SRCS}
#	rm -f ${NAME} ${OBJECTS}

#project dependencies
mhmt-emit.o: ../src/mhmt-emit.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-lz.h ../src/mhmt-optimal.h ../src/mhmt-emit.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-globals.o: ../src/mhmt-globals.c ../src/mhmt-types.h ../src/mhmt-globals.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-hash.o: ../src/mhmt-hash.c ../src/mhmt-types.h ../src/mhmt-hash.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-lz.o: ../src/mhmt-lz.c ../src/mhmt-types.h ../src/mhmt-tb.h ../src/mhmt-lz.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-main.o: ../src/mhmt-main.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-parsearg.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-optimal.o: ../src/mhmt-optimal.c ../src/mhmt-types.h ../src/mhmt-optimal.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-pack.o: ../src/mhmt-pack.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-pack.h ../src/mhmt-hash.h ../src/mhmt-tb.h ../src/mhmt-lz.h ../src/mhmt-optimal.h ../src/mhmt-emit.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-parsearg.o: ../src/mhmt-parsearg.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-parsearg.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-tb.o: ../src/mhmt-tb.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-tb.h Makefile
	${CC} ${CFLAGS} -c $<

mhmt-depack.o: ../src/mhmt-depack.c ../src/mhmt-types.h ../src/mhmt-globals.h ../src/mhmt-depack.h ../src/mhmt-depack-megalz.c ../src/mhmt-depack-hrum.c ../src/mhmt-depack-hrust.c ../src/mhmt-depack-zx7.c Makefile
	${CC} ${CFLAGS} -c $<

%.s: %.o
	${DA} $< >$@

