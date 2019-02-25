\ words for loading input file to mem and fetching bytes from it



\ variables to store state

variable infile
variable insize
variable inpos
variable inmem

\ loads input file to mem and inits fetcher

: input-load ( addr u -- ) \ input is string


	\ some init

	0 dup dup dup   infile !  inmem !  insize !  inpos !



	r/o bin open-file throw

	dup infile !

	file-size throw

	2dup d>s insize !

	2dup d0= if
		." file has zero length!"
		bye
	endif


	2dup
	s" MAX-U" environment?
	0= if
		." couldn't get MAX-U environment var!"
		bye
	endif

	0
	d> if
		." file too large (>2^32-1 bytes)!"
		bye
	endif



	d>s

	allocate throw

	inmem !


	
	inmem @  insize @  infile @  read-file throw

	insize @ <> if
		." can't read all the file!"
		bye
	endif
;



\ gets byte from input (unsigned) or -1 if no (more) bytes

: input-getbyte ( -- d )

	inmem @  0= if
		-1 exit
	endif


	inpos @  insize @  u>= if
		-1 exit
	endif


	inmem @  inpos @  +  c@


	1 inpos +!
;





\ closes file opened by input-load

: input-close ( -- )

	inmem @

	?dup 0<> if
		free throw
	endif


	infile @

	?dup 0<> if
		close-file throw
	endif
;

