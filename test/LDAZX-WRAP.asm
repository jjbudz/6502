; Test LDA Zero Page,X - Wraparound
; Tests: LDA $nn,X wraps at $FF boundary
; Expected: $FF + $02 wraps to $01, not $101
; Result stored at $8000 should be $33
$4000   LDAI #$33
        STAZ $01      ; Store test value at $01
        LDXI #$02     ; X = $02
        LDAZX $FF     ; $FF + $02 = $01 (wraps in zero page)
        STAA $8000
        BRK
