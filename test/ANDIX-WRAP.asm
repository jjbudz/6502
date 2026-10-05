; Test AND Indexed Indirect - Wraparound in zero page pointer calculation
; Tests: AND ($nn,X) when X causes wraparound in zero page address calculation
; Expected: $FE + $02 wraps to $00 in zero page, not $100
; On 6502, zero page indexed wraps: base $FE + X $02 = $00 (not $100)
$4000   LDAI #$34     ; Low byte of data address
        STAZ $00      ; Pointer low byte at $00
        LDAI #$80     ; High byte
        STAZ $01      ; Pointer high byte at $01 -> points to $8034
        LDAI #$AA
        STAA $8034    ; Store test data at $8034
        LDXI #$02     ; X = $02
        LDAI #$FF
        ANDIX $FE     ; ($FE,X) = ($00) because $FE+$02 wraps to $00
        STAA $8000    ; Result should be $FF AND $AA = $AA
        BRK
