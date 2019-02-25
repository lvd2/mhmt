                org #6000

; dehrust1 original external unpacker, 256 bytes. uses stack!
; HL = src
; DE = dst

dehrust:
                ld      ix, 0-12
                add     ix, sp
                push    de
                ld      sp, hl
                pop     bc
                ex      de, hl
                pop     bc
                dec     bc
                add     hl, bc
                ex      de, hl
                pop     bc
                dec     bc
                add     hl, bc
                sbc     hl, de
                add     hl, de
                jr      c, loc_6018
                ld      d, h
                ld      e, l

loc_6018:
                lddr
                ex      de, hl
                ld      d, (ix+#0B)
                ld      e, (ix+#0A)
                ld      sp, hl
                pop     hl
                pop     hl
                pop     hl
                ld      b, 6

loc_6027:
                dec     sp
                pop     af
                ld      (ix+6), a
                inc     ix
                djnz    loc_6027
                exx
                ld      d, #BF
                ld      bc, #1010
                pop     hl

loc_6037:
                dec     sp
                pop     af
                exx

loc_603A:
                ld      (de), a
                inc     de

loc_603C:
                exx

loc_603D:
                add     hl, hl
                djnz    loc_6042
                pop     hl
                ld      b, c

loc_6042:
                jr      c, loc_6037
                ld      e, 1

loc_6046:
                ld      a, #80

loc_6048:
                add     hl, hl
                djnz    loc_604D
                pop     hl
                ld      b, c

loc_604D:
                rla
                jr      c, loc_6048
                cp      3
                jr      c, loc_6059
                add     a, e
                ld      e, a
                xor     c
                jr      nz, loc_6046

loc_6059:
                add     a, e
                cp      4
                jr      z, loc_60B8
                adc     a, #FF
                cp      2
                exx

loc_6063:
                ld      c, a

loc_6064:
                exx
                ld      a, #BF
                jr      c, loc_607D

loc_6069:
                add     hl, hl
                djnz    loc_606E
                pop     hl
                ld      b, c

loc_606E:
                rla
                jr      c, loc_6069
                jr      z, loc_6078
                inc     a
                add     a, d
                jr      nc, loc_607F
                sub     d

loc_6078:
                inc     a
                jr      nz, loc_6087
                ld      a, #EF

loc_607D:
                rrca
                cp      a

loc_607F:
                add     hl, hl
                djnz    loc_6084
                pop     hl
                ld      b, c

loc_6084:
                rla
                jr      c, loc_607F

loc_6087:
                exx
                ld      h, -1
                jr      z, loc_6092
                ld      h, a
                dec     sp
                inc     a
                jr      z, loc_609D
                pop     af

loc_6092:
                ld      l, a
                add     hl, de
                ldir

loc_6096:
                jr      loc_603C
; ----

loc_6098:
                exx
                rrc     d
                jr      loc_603D
; ----

loc_609D:
                pop     af
                cp      #E0
                jr      c, loc_6092
                rlca
                xor     c
                inc     a
                jr      z, loc_6098
                sub     #10

loc_60A9:
                ld      l, a
                ld      c, a
                ld      h, #FF
                add     hl, de
                ldi
                dec     sp
                pop     af
                ld      (de), a
                inc     hl
                inc     de
                ld      a, (hl)
                jr      loc_603A
; ----

loc_60B8:
                ld      a, #80

loc_60BA:
                add     hl, hl
                djnz    loc_60BF
                pop     hl
                ld      b, c

loc_60BF:
                adc     a, a
                jr      nz, loc_60DB
                jr      c, loc_60BA
                ld      a, #FC
                jr      loc_60DE
; ----

loc_60C8:
                dec     sp
                pop     bc
                ld      c, b
                ld      b, a
                ccf
                jr      loc_6064
; ----

loc_60CF:
                cp      #0F
                jr      c, loc_60C8
                jr      nz, loc_6063
                add     a, #F4
                ld      sp, ix
                jr      loc_60EF
; ----

loc_60DB:
                sbc     a, a
                ld      a, #EF

loc_60DE:
                add     hl, hl
                djnz    loc_60E3
                pop     hl
                ld      b, c

loc_60E3:
                rla
                jr      c, loc_60DE
                exx
                jr      nz, loc_60A9
                bit     7, a
                jr      z, loc_60CF
                sub     #EA

loc_60EF:
                ex      de, hl

loc_60F0:
                pop     de
                ld      (hl), e
                inc     hl
                ld      (hl), d
                inc     hl
                dec     a
                jr      nz, loc_60F0
                ex      de, hl
                jr      nc, loc_6096
                ld      hl, #2758
                exx
                ret
