; Test BCD ADC NMOS flag quirk - Z comes from the binary sum
; Tests: SED; $99 + $01 = $00 decimally with carry, but on an NMOS 6502
;        Z reflects the binary sum ($9A, not zero) so Z=0 even though A=$00.
;        N comes from the intermediate $A0 so N=1 as well.
; Expected: A=$00, Z=0, N=1, C=1
; Result stored at $8000 should be $01 (success)
$4000   SED
        CLC
        LDAI #$99
        ADCI #$01
        BNE ok1       ; Z must be clear despite A=$00
        BRK
ok1     BMI ok2       ; N must be set despite A=$00
        BRK
ok2     BCS ok3       ; C must be set
        BRK
ok3     CMPI #$00
        BEQ pass
        BRK
pass    CLD
        LDAI #$01
        STAA $8000
        BRK
