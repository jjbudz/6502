$4000   LDYI #$05
        LDAI #$FB
        STAZ $50
        LDAI #$80
        STAZ $51
        LDAI #$50
        STAA $8100
        LDAI #$50
        CMPIY $50
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
