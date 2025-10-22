$4000   LDXI #$05
        LDAI #$50
        STAZ $50
        LDAI #$50
        CMPZX $4B
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
