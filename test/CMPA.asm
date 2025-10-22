$4000   LDAI #$50
        STAA $8100
        LDAI #$50
        CMPA $8100
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
