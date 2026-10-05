; Test LDX Zero Page,Y - Wraparound
; Tests: LDX $nn,Y wraps at $FF boundary
; Expected: $FE + $03 wraps to $01, not $101
; Result stored at $8000 should be $44
$4000   LDAI #$44
        STAZ $01      ; Store test value at $01
        LDYI #$03     ; Y = $03
        LDXZY $FE     ; $FE + $03 = $01 (wraps in zero page)
        TXA           ; Transfer X to A
        STAA $8000
        BRK
