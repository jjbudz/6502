; Constants (NAME = value), * = as .ORG, binary and character literals,
; and a negative immediate (#-1 is $FF).
* = $4000
ONE  = 1
MASK = %00001111
        LDA #'A'           ; $41
        AND #MASK          ; $01
        CLC
        ADC #ONE           ; $02
        ADC #-1            ; + $FF = $01, carry set
        ADC #$10           ; $01 + $10 + 1 = $12
        STA $8000
        BRK
