; 16-bit by 8-bit division. Divides the word at $40/$41 by the byte at $42; the quotient
; ends up in $43 and the remainder in $44.
dividend = $40
divisor  = $42
quotient = $43
remainder = $44

        * = dividend
        .BYTE $6d, $32, $47

        * = $4000
        LDX #8
        LDA dividend
        STA quotient
        LDA dividend+1
divide: ASL quotient
        ROL A
        CMP divisor
        BCC next
        SBC divisor
        INC quotient
next:   DEX
        BNE divide
        STA remainder
        BRK
