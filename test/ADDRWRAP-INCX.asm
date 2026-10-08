; Effective address $FFF0 + $20 must wrap to $0010 (16-bit address bus),
; not index past the end of 64K memory.
; Read-modify-write: INC must read and write the wrapped address.
$4000   LDAI #$41
        STAZ $10
        LDXI #$20
        INCX $FFF0
        LDAZ $10
        STAA $8000
        BRK
