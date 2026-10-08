; Effective address $FFF0 + $20 must wrap to $0010 (16-bit address bus),
; not index past the end of 64K memory.
$4000   LDXI #$20
        LDAI #$5A
        STAX $FFF0
        LDAZ $10
        STAA $8000
        BRK
