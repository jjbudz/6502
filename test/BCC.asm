; Test BCC (Branch if Carry Clear)
; Tests: BCC rel
; Expected: Branch should be taken after CLC
; Result stored at $8000 should be $01 (success)
$4000   CLC
        BCC pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
