; Test Stack Operations with Full Stack
; Tests: Multiple pushes and pops work correctly
; Expected: LIFO behavior with values preserved
$4000   LDXI #$FF     ; Initialize stack pointer
        TXS
        LDAI #$11
        PHA           ; Push $11
        LDAI #$22
        PHA           ; Push $22
        LDAI #$33
        PHA           ; Push $33
        PLA           ; Pull $33
        STAA $10
        PLA           ; Pull $22
        STAA $11
        PLA           ; Pull $11
        STAA $12
        LDAZ $10      ; Should be $33
        CMPI #$33
        BNE $4030
        LDAZ $11      ; Should be $22
        CMPI #$22
        BNE $4030
        LDAZ $12      ; Should be $11
        CMPI #$11
        BNE $4030
        LDAI #$01     ; Success
        STAA $8000
        BRK
$4030   BRK           ; Failure
