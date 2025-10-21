        ;; Test timing simulation - simple program to validate ticker works
        ;; Expected result: $9000 = 0x42
$4000   LDAI #$42
        STAA $9000
        NOP
        NOP
        NOP
        BRK
