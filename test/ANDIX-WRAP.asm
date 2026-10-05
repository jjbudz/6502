; Test AND Indexed Indirect - Wraparound in pointer calculation
; Tests: AND ($nn,X) wraps in zero page when calculating pointer
; Expected: $FE + $04 = $02 in zero page for pointer lookup
; Result: AND operation uses correct pointer
$4000   LDAI #$34     ; Low byte of data address
        STAZ $02
        LDAI #$80     ; High byte
        STAZ $03      ; Pointer at $02-$03 points to $8034
        LDAI #$AA
        STAA $8034    ; Store data value
        LDXI #$04     ; X = $04, so ($FA,X) uses $FE+$04=$02 (wrap)
        LDAI #$FF
        ANDIX $FA     ; AND with value at ($FA,X) = ($02) = $8034
        STAA $8000    ; Result should be $FF AND $AA = $AA
        BRK
