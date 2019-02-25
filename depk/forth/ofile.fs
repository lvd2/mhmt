\ output file management



variable ofileid



\ make name for output file

: ofile-mkname ( caddr u -- caddr u ) \ makes new name by appending ".dpk"

	dup 4 chars +

	here swap allot align   ( caddr-old u-old caddr-new )

	swap ( caddr-old caddr-new u-old )

	2dup >r >r

	cmove

	r> r> ( caddr-new u-old )

	2dup +

	s" .dpk" ( caddr-to caddr-from u-dpk )

	>r swap r> cmove


	4 chars + ( caddr-new u-new )
;





\ create output file

: ofile-create ( caddr u -- )

	0 ofile !

	ofile-mkname

	w/o bin create-file throw

	ofileid !
;



\ write to output file

: ofile-write ( caddr u -- )

	ofileid @  write-file throw
;




\ close output file

: ofile-close

	ofileid @

	?dup 0<> if
		close-file throw
	endif
;

