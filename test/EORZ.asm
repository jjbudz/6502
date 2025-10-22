; Test EOR (Exclusive OR) instruction - zero page mode
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
        ; Test 1: 0xFF XOR 0xFF = 0x00
        LDAI  #$FF
        EORZ  $20
        CMPI  #$00
        BNE   fail1
        ; Test 2: 0xAA XOR 0x55 = 0xFF
        LDAI  #$AA
        EORZ  $22
        CMPI  #$FF
        BNE   fail2
        ; Test 3: 0x55 XOR 0xAA = 0xFF
        LDAI  #$55
        EORZ  $21
        CMPI  #$FF
        BNE   fail3
        JMP   pass
fail1   LDAI  #$F1
        STAA  $8000
        BRK
fail2   LDAI  #$F2
        STAA  $8000
        BRK
fail3   LDAI  #$F3
        STAA  $8000
        BRK
pass    LDAI  #$01
        STAA  $8000
        BRK
