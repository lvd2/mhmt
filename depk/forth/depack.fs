\ depacker itself



variable obuf-ptr



\ inits depacker

: depack-init

	0 obuf-ptr !

;



\ puts byte to obuf

: depack-putbyte ( c -- )

	obuf-ptr @  swap over
	obuf @  +
	c!

	1+ obuf-size 1- and

	dup obuf-ptr !

	0= if
		obuf @  obuf-size  ofile-write
	endif
;


\ gets byte from inmem. returns either unsigned char or -1

: depack-getbyte ( -- d )
	input-getbyte

	dup 0< if
		." unexpected end of input file!" cr
		bye
	endif
;


\ writes remaining of obuf to file

: depack-cleanup ( -- )

	obuf-ptr @ dup 0<> if
		obuf @  swap  ofile-write
	endif
;








\ bitstream functions

variable dbits
variable dbits-num

\ byte from input to bits


\ put byte to bits buffer, init everything

: depack-initbits ( c -- )
	depack-getbyte dbits !
	8 dbits-num !
;


\ get 1 bit
: depack-getbit ( -- u )
	dbits-num @
	0= if
		depack-initbits
	endif

	dbits dup @
	dup 2* rot !

	128 and 0<>

	-1 dbits-num +!
;



\ get number of bits aligned to LSB

: depack-getbits ( u -- u )

	0 swap 0 ?do
		
		depack-getbit
		1 and
		swap 2* or

	loop
;








\ copies byte from input to output
: depack-copybyte
	depack-getbyte depack-putbyte
;





\ repeats given number of bytes in obuf from given negative displacement

: depack-repeat ( usize sdisplacement -- )

	swap 0
	?do
		dup  obuf-ptr @  +
		obuf-size 1- and
		obuf @  +  c@

		depack-putbyte
	loop

	drop
;








\ fetch bytewise displacement
: depack-bytedisp

	depack-getbyte -256 or
;



\ fetch big displacement from input stream

: depack-bigdisp ( -- sdisplacement )
	
	depack-getbit

	if
		4 depack-getbits
		-17 +  8 lshift  depack-getbyte +
	else
		depack-bytedisp
	endif
;


\ fetch big length from input stream
\ returns either len>0 or 0 if stop-code encountered

: depack-biglen ( -- ulen )

	1
	begin
		depack-getbit 0=
	while
		1+
	repeat

	dup 9 = if
		drop 0 exit
	endif


	dup 7 > if
		." wrong big length code!" cr
		bye
	endif


	1 over lshift swap
	depack-getbits +
	2 +

	dup 255 > if
		." wrong big length: more than 255!" cr
		bye
	endif
;





\ interpret remaining bits after initial bit in code
\ returns true if end-code encountered, else false

: depack-dobits ( -- f )

	2 depack-getbits

	case

	0 of
		1
		3 depack-getbits -8 or
	endof

	1 of
		2
		depack-bytedisp
	endof

	2 of
		3
		depack-bigdisp
	endof
	
	3 of
		depack-biglen
		dup 0> if
			depack-bigdisp
		else
			invert exit
		endif
	endof
	
	endcase


	depack-repeat
	0
;








\ main depack loop

: depack-loop ( -- )

	begin
		depack-getbit
		if
			depack-copybyte 0
		else
			depack-dobits
		endif
	until
;



\ depacker - uses input-getbyte and ofile-write, uses obuf

: depack ( -- )

	depack-init


	\ first byte copied as-is.

	depack-copybyte


	\ second byte inits bitstream

	depack-initbits


	\ main depack loop

	depack-loop



	depack-cleanup
;

