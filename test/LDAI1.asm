; Test LDA (Load Accumulator) - Immediate addressing mode
; Tests: LDA #$imm (decimal format)
; Expected: Load 0, then load $7F into accumulator
; Result stored at $8000 should be $7F
$4000   LDAI  #000
        STAA  $8000
        LDAI  #$7f
        STAA  $8000
        BRK 
