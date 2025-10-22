$4000   CLC
        LDXI #$05
        LDAI #$AA
        STAA $8100
        RORX $80FB
        LDAA $8100
        STAA $8000
        BRK
