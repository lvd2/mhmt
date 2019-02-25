\ output buffer procs



8192 constant obuf-size \ must be 2^n

variable obuf-mem
variable obuf


\ initialize output buffer

: obuf-init

	0 dup   obuf ! obuf-mem !


	obuf-size 2* allocate throw

	dup obuf-mem !


	obuf-size 1- dup

	rot + swap invert and


	obuf !
;




: obuf-release

	obuf-mem @

	?dup 0<> if
		free throw
	endif
;

