\ parses argument and either returns it as a string
\ or prints help and aborts program

: argparse ( -- addr u )

	next-arg


	\ if no args -- print help and exit

	2dup d0=
	if
		prhelp

		bye
	endif


	\ compare arg to either -h or --help and then print help

	2dup 2dup

	s" -h" compare 0= -rot

	s" --help" compare 0=

	or

	if
		prhelp

		bye
	endif

	\ if neither of above, leave string ( addr u ) on stack and return

;

