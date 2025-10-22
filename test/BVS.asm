$4000   LDAI #$7F
        ADCI #$01
        BVS pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
