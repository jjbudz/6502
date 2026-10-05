; Test STX Zero Page,Y - Wraparound
; Tests: STX $nn,Y wraps at $FF boundary
; Expected: Stores to $00 when base is $F8 and Y is $08
; Verification: Reading from $00 should get stored value
$4000   LDYI #$08     ; Y = $08
        LDXI #$66
        STXZY $F8     ; $F8 + $08 = $00 (wraps in zero page)
        LDAZ $00      ; Verify it was stored at $00
        STAA $8000
        BRK
