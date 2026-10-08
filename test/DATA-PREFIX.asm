; .DATA accepts $-prefixed and bare hex, stores 4-digit values low byte
; first, and stops at a trailing comment instead of emitting it as data.
$40     .DATA $12 34 $AB $5678 ; trailing comment
$4000   LDAZ $40
        CMPI #$12
        BNE fail
        LDAZ $41
        CMPI #$34
        BNE fail
        LDAZ $42
        CMPI #$AB
        BNE fail
        LDAZ $43
        CMPI #$78
        BNE fail
        LDAZ $44
        CMPI #$56
        BNE fail
        LDAZ $45
        CMPI #$00
        BNE fail
        LDAI #$01
        STAA $8000
fail    BRK
