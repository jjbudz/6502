; Test Zero Flag - ADC Producing Zero
; Tests: $FF + $01 = $00 sets Z=1, C=1, V=0
; Result stored at $8000 should be $01 (success)
$4000   CLC
        LDAI #$FF
        ADCI #$01     ; $100 truncates to $00
        BEQ ok1       ; Z must be set
        BRK
ok1     BCS ok2       ; C must be set
        BRK
ok2     BVC pass      ; V must be clear (-1 + 1 does not overflow)
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
