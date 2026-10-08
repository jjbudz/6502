; Standard syntax: ADC in every addressing mode, each adding to a running total.
; Also exercises LDX/LDY/LDA immediate and STA zero page / absolute.
$4000   LDX #$02
        LDY #$03
        LDA #$10
        STA $50          ; zero page
        STA $52          ; read back below as $50,X
        STA $4100        ; absolute; also the target of both indirect modes
        STA $4102        ; read back as $4100,X
        STA $4103        ; read back as $4100,Y
        LDA #$00
        STA $62          ; pointer at $62/$63 = $4100, reached as ($60,X)
        LDA #$41
        STA $63
        LDA #$FD
        STA $70          ; pointer at $70/$71 = $40FD, reached as ($70),Y = $4100
        LDA #$40
        STA $71
        CLC
        LDA #$01         ; immediate            A = $01
        ADC $50          ; zero page            A = $11
        ADC $50,X        ; zero page,X  ($52)   A = $21
        ADC $4100        ; absolute             A = $31
        ADC $4100,X      ; absolute,X   ($4102) A = $41
        ADC $4100,Y      ; absolute,Y   ($4103) A = $51
        ADC ($60,X)      ; (indirect,X) ($62 -> $4100) A = $61
        ADC ($70),Y      ; (indirect),Y ($40FD + 3)    A = $71
        STA $8000
        BRK
