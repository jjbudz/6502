; Test ADC Sign Bit Transition at $7F
; Tests: $7F + $01 = $80 sets V=1, N=1, C=0
; Result stored at $8000 should be $01 (success)
$4000   CLC
        LDAI #$7F     ; Largest positive signed byte
        ADCI #$01     ; -> $80: positive to negative = overflow
        BVS ok1       ; V must be set
        BRK
ok1     BMI ok2       ; N must be set
        BRK
ok2     BCC ok3       ; C must be clear
        BRK
ok3     CMPI #$80
        BEQ pass
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
