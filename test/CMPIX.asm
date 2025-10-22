$4000   LDXI #$05
        LDAI #$00
        STAZ $50
        LDAI #$81
        STAZ $51
        LDAI #$50
        STAA $8100
        LDAI #$50
        CMPIX $4B
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
