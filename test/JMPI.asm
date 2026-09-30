; Test JMP (Jump) - Indirect addressing mode
; Tests: JMP ($addr)
; Expected: Jump to address stored at $0050 (which is $4010)
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$10
        STAZ $50
        LDAI #$40
        STAZ $51
        JMPI $0050
        LDAI #$00
        STAA $8000
        BRK
$4010   LDAI #$01
        STAA $8000
        BRK
