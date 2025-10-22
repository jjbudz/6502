$4000   LDXI #$05
        LDAI #$00
        STAZ $50
        LDAI #$81
        STAZ $51
        LDAI #$55
        STAA $8100
        LDAI #$FF
        ANDIX $4B
        STAA $8000
        BRK
