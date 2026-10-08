; Standard syntax: zero page vs absolute selection.
;   $50     -> zero page (2-byte instruction)
;   $0050   -> absolute, because it was written with four digits
;   EARLY   -> zero page, label defined earlier at an address under $100
;   LATE    -> absolute, label defined later in the file
; All four read the same byte, so the result checks correctness; the
; golden file checks the encoding.
$0080
EARLY   .DATA $00
$4000   LDA #$07
        STA $50
        STA EARLY
        STA LATE
        LDA #$00
        CLC
        ADC $50
        ADC $0050
        ADC EARLY
        ADC LATE
        STA $8000
        BRK
$4100
LATE    .DATA $00
