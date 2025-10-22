; Test EOR (Exclusive OR) instruction - absolute indexed by Y
$4000   CLC
        LDAI  #$00
        STAA  $8000
        ; Set up test values in memory
        LDAI  #$FF
        STAA  $1020
        LDAI  #$AA
        STAA  $1021
        LDAI  #$55
        STAA  $1022
        ; Set Y register
        LDYI  #$01
        ; Test 1: 0xAA XOR 0xAA = 0x00
        LDAI  #$AA
        EORY  $1020     ; $1020 + Y = $1021, value is 0xAA
        CMPI  #$00
        BNE   fail1
        ; Test 2: 0xFF XOR 0x55 = 0xAA
        LDYI  #$02
        LDAI  #$FF
        EORY  $1020     ; $1020 + Y = $1022, value is 0x55
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
