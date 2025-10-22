; Test EOR (Exclusive OR) instruction - zero page indexed by X
$4000   CLC
        LDAI  #$00
        STAA  $8000
        ; Set up test values in zero page
        LDAI  #$FF
        STAZ  $20
        LDAI  #$AA
        STAZ  $21
        LDAI  #$55
        STAZ  $22
        ; Set X register
        LDXI  #$01
        ; Test 1: 0xAA XOR 0xAA = 0x00
        LDAI  #$AA
        EORZX $20       ; $20 + X = $21, value is 0xAA
        CMPI  #$00
        BNE   fail1
        ; Test 2: 0xFF XOR 0x55 = 0xAA
        LDXI  #$02
        LDAI  #$FF
        EORZX $20       ; $20 + X = $22, value is 0x55
        CMPI  #$AA
        BNE   fail2
        JMP   pass
fail1   LDAI  #$F1
        STAA  $8000
        BRK
fail2   LDAI  #$F2
        STAA  $8000
        BRK
pass    LDAI  #$01
        STAA  $8000
        BRK
