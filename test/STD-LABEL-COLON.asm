; Standard syntax: labels with a trailing colon may be indented, and
; source is case-insensitive. Counts X down from 5, adding 2 to A each time.
$4000   ldx #5
        lda #0
        clc
  loop: adc #2
        dex
        bne loop
        sta $8000
        brk
