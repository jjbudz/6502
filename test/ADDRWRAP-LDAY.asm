; Effective address $FFF0 + $20 must wrap to $0010 (16-bit address bus),
; not index past the end of 64K memory.
$4000   LDAI #$5A
        STAZ $10
        LDYI #$20
        LDAI #$00
        LDAY $FFF0
        STAA $8000
        BRK
