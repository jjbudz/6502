; Cycle counting, including the state-dependent penalties: +1 for a read
; through abs,X / abs,Y / (zp),Y that crosses a page, +1 for a taken branch
; and +1 more when it lands on another page. Stores and read-modify-write
; instructions take no penalty. The running total is in the comments; the
; test checks the final count.
* = $4000
        LDX #$20        ;  2   2
        LDY #$20        ;  2   4
        LDA $40F0,X     ;  5   9  page crossed
        LDA $4010,X     ;  4  13
        STA $90F0,X     ;  5  18  store: never a penalty
        LDA $40F0,Y     ;  5  23  page crossed
        LDA #$F0        ;  2  25
        STA $10         ;  3  28
        LDA #$40        ;  2  30
        STA $11         ;  3  33
        LDA ($10),Y     ;  6  39  $40F0+$20 crosses
        LDY #$00        ;  2  41
        LDA ($10),Y     ;  5  46
        INC $90F0,X     ;  7  53  read-modify-write: never a penalty
        CLC             ;  2  55
        BCS near        ;  2  57  not taken
        BCC near        ;  3  60  taken, offset 0, same page
near    JMP far         ;  3  63

* = $40FA
far     CLC             ;  2  65
        BCC over        ;  4  69  taken onto the next page

* = $4105
over    LDA #$01        ;  2  71
        STA $8000       ;  4  75
        BRK             ;  7  82
