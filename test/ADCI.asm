; Test 1: Basic ADC with carry clear (0+1+0=1)
$4000   CLC
        LDAI  #000
        STAA  $8000
        ADCI  #001
        CMPI  #001
        BNE   f1
        JMP   t2
f1      JMP   fail1
; Test 2: ADC with overflow (1+255+0=256, C=1)
t2      CLC
        ADCI  #$FF
        BCC   f2
        CMPI  #000
        BNE   f3
        JMP   t3
f2      JMP   fail2
f3      JMP   fail3
; Test 3: 127+128+0=255, no overflow, no carry, sign set
t3      LDAI  #$7F
        CLC
        ADCI  #$80
        BCS   f4
        BVS   f5
        BPL   f6
        CMPI  #$FF
        BNE   f7
        JMP   t4
f4      JMP   fail4
f5      JMP   fail5
f6      JMP   fail6
f7      JMP   fail7
; Test 4: 80+80+0=160, overflow set, sign set
t4      LDAI  #$50
        CLC
        ADCI  #$50
        BCS   f8
        BVC   f9
        BPL   fA
        CMPI  #$A0
        BNE   fB
        JMP   t5
f8      JMP   fail8
f9      JMP   fail9
fA      JMP   failA
fB      JMP   failB
; Test 5: 80+208+0=32 with carry
t5      LDAI  #$50
        CLC
        ADCI  #$D0
        BCC   fC
        BVS   fD
        BMI   fE
        CMPI  #$20
        BNE   fF
        JMP   t6
fC      JMP   failC
fD      JMP   failD
fE      JMP   failE
fF      JMP   failF
; Test 6: Negative+Negative with overflow (-128+-128=0)
t6      LDAI  #$80
        CLC
        ADCI  #$80
        BCC   f10
        BVC   f11
        BPL   f12a
        JMP   f12
f10     JMP   fail10
f11     JMP   fail11
f12     JMP   fail12
f12a    CMPI  #$00
        BNE   f13
        JMP   t7
f13     JMP   fail13
; Test 7: Carry in (1+255+1=257, C=1)
t7      LDAI  #$01
        SEC
        ADCI  #$FF
        BCC   f14
        CMPI  #$01
        BNE   f15
        JMP   t8
f14     JMP   fail14
f15     JMP   fail15
; Test 8: Carry causes overflow (64+64+1=129=-127)
t8      LDAI  #$40
        SEC
        ADCI  #$40
        BCS   f16
        BVC   f17
        BPL   f18
        CMPI  #$81
        BNE   f19
        JMP   pass
f16     JMP   fail16
f17     JMP   fail17
f18     JMP   fail18
f19     JMP   fail19
fail1   LDAI  #$01
        JMP   done
fail2   LDAI  #$02
        JMP   done
fail3   LDAI  #$03
        JMP   done
fail4   LDAI  #$04
        JMP   done
fail5   LDAI  #$05
        JMP   done
fail6   LDAI  #$06
        JMP   done
fail7   LDAI  #$07
        JMP   done
fail8   LDAI  #$08
        JMP   done
fail9   LDAI  #$09
        JMP   done
failA   LDAI  #$0A
        JMP   done
failB   LDAI  #$0B
        JMP   done
failC   LDAI  #$0C
        JMP   done
failD   LDAI  #$0D
        JMP   done
failE   LDAI  #$0E
        JMP   done
failF   LDAI  #$0F
        JMP   done
fail10  LDAI  #$10
        JMP   done
fail11  LDAI  #$11
        JMP   done
fail12  LDAI  #$12
        JMP   done
fail13  LDAI  #$13
        JMP   done
fail14  LDAI  #$14
        JMP   done
fail15  LDAI  #$15
        JMP   done
fail16  LDAI  #$16
        JMP   done
fail17  LDAI  #$17
        JMP   done
fail18  LDAI  #$18
        JMP   done
fail19  LDAI  #$19
        JMP   done
pass    LDAI  #$FF
done    STAA  $8000
        BRK


