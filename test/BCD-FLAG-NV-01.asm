; Test BCD ADC NMOS flag quirk - N and V from the intermediate sum
; Tests: SED; $79 + $01 = $80 decimally, and on an NMOS 6502 the
;        low-nibble adjust makes the intermediate sum $80, so N=1 and V=1
;        even though the decimal result is a plain carry into the tens digit
; Expected: A=$80, N=1, V=1, C=0
; Result stored at $8000 should be $01 (success)
$4000   SED
        CLC
        LDAI #$79
        ADCI #$01
        BMI ok1       ; N must be set
        BRK
ok1     BVS ok2       ; V must be set
        BRK
ok2     BCC ok3       ; C must be clear
        BRK
ok3     CMPI #$80
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
