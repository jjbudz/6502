$4000   LDAI #$50
        STAZ $50
        LDYI #$50
        CPYZ $50
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
