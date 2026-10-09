; .EQU as a synonym for =, and a string inside .BYTE.
seven   .EQU 7
result  .EQU $8000
        * = $4000
        LDA msg+1          ; 'B' = $42
        CLC
        ADC #seven         ; $49
        ADC msg+2          ; + 0
        STA result
        BRK
msg     .BYTE "AB", 0
