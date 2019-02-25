;Original Z80 depacker for megalz V4 packed files (C) fyrex^mhm
;8080 port by AlCo&Shiru (for use in RAM only)


	module megalz

unpack
	ld a,#80	;these two operations could be removed if
	ld (a2),a	;the packer called only once (5 bytes)
ms
	ld b,(hl)
	ex de,hl
	ld (hl),b
	ex de,hl
	inc hl
	inc de
m0
	ld      bc,#2ff
m1
	ld (a1),a
a2=$+1
	ld a,#80
m1x
	add     a,a
	jp      nz,m2
	ld      a,(hl)
	inc     hl
	rla
m2
	push de
	ld e,a
	ld a,c
	rla
	ld c,a
	ld a,e
	pop de
	jp      nc,m1x
	ld (a2),a ;CY=1
a1=$+1
	ld a,#80
	dec b
	jp nz,x2
	ld a,c
	rla
	ld a,c
	rra
	ld c,a
	ld      a,2
	jp      c,n1
	inc     a
	inc     c
	jp      z,n2
	ld      bc,#33f
	jp      m1

x2
	dec b
	jp nz,x3
	ld b,a
	ld a,c
	and a
	rra
	ld c,a
	ld a,b
	jp      c,ms
	ld b,1
	jp      m1
x6
	add     a,c
n2
	ld      bc,#4ff
	jp      m1
n1
	inc     c
	jp      nz,m4
    ;scf		;somehow works without this
	ld (a1),a
	ld a,(a2) ;CY=1
	inc     b
n5
	ld (tmp1),a
	ld a,c
	rra
	ld c,a
	ret     c
	ld a,b
	rla
	ld b,a
tmp1=$+1
	ld a,0
	add     a,a
	jp      nz,n6
	ld      a,(hl)
	inc     hl
	rla
n6
	jp      nc,n5
	ld (a2),a
	ld a,(a1)
	add     a,b
	ld      b,6
	jp      m1
x3
	dec b
	jp nz,x4
	ld      a,1
	jp      m3
x4
	dec b
	jp nz,x5
	inc     c
	jp      nz,m4
	ld      bc,#51f
	jp      m1
x5
	dec b
	jp nz,x6
	ld      b,c
m4
	ld      c,(hl)
	inc     hl
m3
	dec     b
	push    hl
	ld      l,c
	ld      h,b
	add     hl,de
	ld      c,a
loop
	ld a,(hl)
	ld (de),a
	inc hl
	inc de
	dec c
	jp nz,loop
	pop     hl
	jp      m0
end_dec40

	endmodule
