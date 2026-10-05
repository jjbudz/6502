; Test Stack LIFO Ordering
; Tests: Three pushes then three pulls return values in reverse order
; Result stored at $8000 should be $01 (success)
$4000   LDXI #$FF
        TXS
        LDAI #$11
        PHA
        LDAI #$22
        PHA
        LDAI #$33
        PHA
        PLA           ; Should be $33
        CMPI #$33
        BNE fail
        PLA           ; Should be $22
        CMPI #$22
        BNE fail
        PLA           ; Should be $11
        CMPI #$11
        BNE fail
        LDAI #$01
        STAA $8000
        BRK
fail    BRK
