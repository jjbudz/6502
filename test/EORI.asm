; Test EOR (Exclusive OR) instruction - immediate mode
; EOR should perform bitwise XOR operation
$4000   CLC
        LDAI  #$00
        STAA  $8000
        ; Test 1: 0xFF XOR 0xFF = 0x00
        LDAI  #$FF
        EORI  #$FF
        CMPI  #$00
        BNE   fail1
        ; Test 2: 0xFF XOR 0x00 = 0xFF
        LDAI  #$FF
        EORI  #$00
        CMPI  #$FF
        BNE   fail2
        ; Test 3: 0xAA XOR 0x55 = 0xFF
        LDAI  #$AA
        EORI  #$55
        CMPI  #$FF
        BNE   fail3
        ; Test 4: 0x0F XOR 0xF0 = 0xFF
        LDAI  #$0F
        EORI  #$F0
        CMPI  #$FF
        BNE   fail4
        ; Test 5: Check zero flag set
        LDAI  #$55
        EORI  #$55
        BNE   fail5     ; Should be zero
        ; Test 6: Check sign flag set
        LDAI  #$00
        EORI  #$80      ; Result should be 0x80 (negative)
        BMI   pass      ; Should jump if sign bit set
fail1   LDAI  #$F1
        STAA  $8000
        BRK
fail2   LDAI  #$F2
        STAA  $8000
        BRK
fail3   LDAI  #$F3
        STAA  $8000
        BRK
fail4   LDAI  #$F4
        STAA  $8000
        BRK
fail5   LDAI  #$F5
        STAA  $8000
        BRK
pass    LDAI  #$01
        STAA  $8000
        BRK
