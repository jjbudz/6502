; Test ASL on $80 - Shift Out Sign Bit
; Tests: ASL A with $80 gives $00, sets C=1, Z=1, N=0
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$80
        ASL           ; $80 << 1 = $00, bit 7 into carry
        BCS ok1       ; C must be set
        BRK
ok1     BEQ ok2       ; Z must be set
        BRK
ok2     BPL pass      ; N must be clear
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
