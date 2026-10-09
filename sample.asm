; Count down X from the value stored at $40.
        * = $40
        .BYTE $06, $55, $AA, $AB

        * = $4000
        LDX $40
loop:   DEX
        BNE loop
        BRK
