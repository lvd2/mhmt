; -----------------------------------------------------------------------------
; ZX7 decoder by Einar Saukas, Antonio Villena & Metalbrain
; "Standard" version
; adapted to i8080 (still Z80 mnemonics!) by lvd^mhm
; -----------------------------------------------------------------------------
; Parameters:
;   HL: source address (compressed data)
;   DE: destination address (decompressing)
; -----------------------------------------------------------------------------

dzx7:
;        ld      a, $80
	ld	a,#80
	ld	(dzx7_bits),a

dzx7s_copy_byte_loop:
;        ldi                             ; copy literal byte
	ld	a,(hl)
	inc	hl
	ld	(de),a
	inc	de

dzx7s_main_loop:
        call    dzx7s_next_bit
;        jr      nc, dzx7s_copy_byte_loop ; next bit indicates either literal or sequence
	jp	nc,dzx7s_copy_byte_loop

; determine number of bits used for length (Elias gamma coding)
        push    de
        ld      bc,0
        ld      d,b
dzx7s_len_size_loop:
        inc     d
        call    dzx7s_next_bit
;        jr      nc, dzx7s_len_size_loop
	jp	nc,dzx7s_len_size_loop


; determine length
dzx7s_len_value_loop:
        call    nc,dzx7s_next_bit
;        rl      c
 ;       rl      b
  ;      jr      c, dzx7s_exit           ; check end marker
	ld	a,c
	rla
	ld	c,a
	ld	a,b
	rla
	ld	b,a
	jp	nc,dzx7s_noexit
	pop	de
	ret
dzx7s_noexit

        dec     d
   ;     jr      nz, dzx7s_len_value_loop
	jp	nz,dzx7s_len_value_loop
        inc     bc                      ; adjust length

; determine offset
;        ld      e,(hl)                 ; load offset flag (1 bit) + offset value (7 bits)
 ;       inc     hl
;        defb    $cb, $33                ; opcode for undocumented instruction "SLL E" aka "SLS E"
;	sli	e
	ld	a,(hl)
	inc	hl
	scf
	rla
	ld	e,a

;        jr      nc, dzx7s_offset_end    ; if offset flag is set, load 4 extra bits
	jp	nc,dzx7s_offset_end

;        ld      d, $10                  ; bit marker to load 4 bits
	ld	d,#10
dzx7s_rld_next_bit:
        call    dzx7s_next_bit
;        rl      d                       ; insert next bit into D
	ld	a,d
	rla
	ld	d,a

;        jr      nc, dzx7s_rld_next_bit  ; repeat 4 times, until bit marker is out
	jp	nc,dzx7s_rld_next_bit

        inc     d                       ; add 128 to DE

;        srl	d			; retrieve fourth bit from D
	ld	a,d
	or	a
	rra
	ld	d,a

dzx7s_offset_end:
;        rr      e                       ; insert fourth bit into E
	ld	a,e
	rra
	ld	e,a

; copy previous sequence
        ex      (sp),hl                ; store source, restore destination
        push    hl                      ; store destination

	;CY=1 here!
;        sbc     hl, de                  ; HL = destination - offset - 1

	ld	a,l
	sbc	a,e
	ld	l,a
	ld	a,h
	sbc	a,d
	ld	h,a

        pop     de                      ; DE = destination

;        ldir
dzx7s_ldir:
	ld	a,(hl)
	inc	hl
	ld	(de),a
	inc	de
	dec	bc
	ld	a,b
	or	c
	jp	nz,dzx7s_ldir


;dzx7s_exit:
        pop     hl                      ; restore source address (compressed data)

;        jr      nc, dzx7s_main_loop
	jp	dzx7s_main_loop



dzx7s_next_bit:
;        add     a, a                    ; check next bit
 ;       ret     nz                      ; no more bits left?
  ;      ld      a, (hl)                 ; load another group of 8 bits
   ;     inc     hl
    ;    rla
     ;   ret

dzx7_bits=$+1
	ld	a,#3e
	add	a,a
	ld	(dzx7_bits),a
	ret	nz
	ld	a,(hl)
	inc	hl
	rla
	ld	(dzx7_bits),a
	ret

; -----------------------------------------------------------------------------
