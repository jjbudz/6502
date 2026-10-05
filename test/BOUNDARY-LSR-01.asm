; Test LSR on $01 - Shift Out Least Significant Bit
; Tests: LSR A with $01 gives $00, sets C=1, Z=1
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$01
        LSR           ; $01 >> 1 = $00, bit 0 into carry
        BCS ok1       ; C must be set
        BRK
ok1     BEQ pass      ; Z must be set
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
