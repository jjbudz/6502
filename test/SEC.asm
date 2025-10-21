$4000   CLC
        BCS fail1
        SEC
        BCC fail2
        JMP pass
fail1   LDAI #$FF
        STAA $8000
        BRK
fail2   LDAI #$FE
        STAA $8000
        BRK
pass    LDAI #001
        STAA $8000
        BRK
