; Test ADC Simple - Just Check Result
; Tests: $40 + $40 should give $80
$4000   CLC
        LDAI #$40
        ADCI #$40
        STAA $8000
        BRK
