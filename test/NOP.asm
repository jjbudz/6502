$4000   NOP
        JMP pass
fail1   LDAI  #$FF
        STAA  $8000
        BRK
pass    LDAI  #001
        STAA  $8000
        BRK
