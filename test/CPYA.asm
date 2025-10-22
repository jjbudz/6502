$4000   LDAI #$50
        STAA $8100
        LDYI #$50
        CPYA $8100
        BEQ pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
