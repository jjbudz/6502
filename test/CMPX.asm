$4000   LDXI #$05
        LDAI #$50
        STAA $8100
        LDAI #$50
        CMPX $80FB
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
