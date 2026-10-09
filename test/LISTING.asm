; Exercises every kind of listing row: comment, constant, .ORG, label on
; its own, instruction, long data line, and a label with a colon.
BASE = $50
        .ORG $4000
start
        LDA #<table
        STA BASE
        LDA #>table
        STA BASE+1
        BRK
table:  .BYTE 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
msg     .TEXT "Hi", 0
        * = $FFFC
        .WORD start
