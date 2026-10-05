; Test STY Zero Page,X - Wraparound
; Tests: STY $nn,X wraps at $FF boundary
; Expected: Stores to $03 when base is $FD and X is $06
; Verification: Reading from $03 should get stored value
$4000   LDXI #$06     ; X = $06
        LDYI #$77
        STYZX $FD     ; $FD + $06 = $03 (wraps in zero page)
        LDAZ $03      ; Verify it was stored at $03
        STAA $8000
        BRK
