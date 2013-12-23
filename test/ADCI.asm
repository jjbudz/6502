$4000   CLC
        LDAI  #00
        STAA  $8000
        ADCI  #01
        CMPI  #01
        BNE   fail1
        ADCI  #$FF
        CLC
        CMPI  #00
        BNE   fail2
        BCS   fail3
        LDAI  #$7F
        ADCI  #$80
        CMPI  #$FF
        BNE   fail4
        BCS   fail5
        JMP   pass
fail1   LDAI  #$FF
        STAA  $8000
        BRK
fail2   LDAI  #$FE
        STAA  $8000
        BRK
fail3   LDAI  #$FD
        STAA  $8000
        BRK
fail4   LDAI  #$FC
        STAA  $8000
        BRK
fail5   LDAI  #$FB
        STAA  $8000
        BRK
pass    LDAI  #01
        STAA  $8000
        BRK


