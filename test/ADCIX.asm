$4000   CLC
        LDXI #$05
        LDAI #$00
        STAZ $50
        LDAI #$81
        STAZ $51
        LDAI #$50
        STAA $8100
        LDAI #$30
        ADCIX $4B
        STAA $8000
        BRK
