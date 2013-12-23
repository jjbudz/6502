$4000   SEC
        BCC fail1
        CLC
        BCS fail2
        JMP pass
fail1   LDAI  #FF
        STAA  $8000
        BRK
fail2   LDAI  #FE
        STAA  $8000
        BRK
pass    LDAI  #01
        STAA  $8000
        BRK
