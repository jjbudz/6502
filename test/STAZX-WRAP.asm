; Test STA Zero Page,X - Wraparound
; Tests: STA $nn,X wraps at $FF boundary
; Expected: Stores to $02 when base is $FF and X is $03
; Verification: Reading from $02 should get stored value
$4000   LDXI #$03     ; X = $03
        LDAI #$55
        STAZX $FF     ; $FF + $03 = $02 (wraps in zero page)
        LDAZ $02      ; Verify it was stored at $02
        STAA $8000
        BRK
