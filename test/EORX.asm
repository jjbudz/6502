; Test EOR (Exclusive OR) instruction - absolute indexed by X
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
        ; Set X register
        LDXI  #$01
        ; Test 1: 0xAA XOR 0xAA = 0x00
        LDAI  #$AA
        EORX  $1020     ; $1020 + X = $1021, value is 0xAA
        CMPI  #$00
        BNE   fail1
        ; Test 2: 0xFF XOR 0x55 = 0xAA
        LDXI  #$02
        LDAI  #$FF
        EORX  $1020     ; $1020 + X = $1022, value is 0x55
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
