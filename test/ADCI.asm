$4000   CLC
        LDAI  #000
        STAA  $8000
        ADCI  #001
        CMPI  #001
        BNE   fail1
        CLC
        ADCI  #$FF
        CMPI  #000
        BNE   fail2
        BCC   fail3
        LDAI  #$7F
        CLC
        ADCI  #$80
        CMPI  #$FF
        BNE   fail4
        BCC   fail5
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
pass    LDAI  #001
        STAA  $8000
        BRK


