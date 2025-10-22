$4000   LDAI #$01
        BPL pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
