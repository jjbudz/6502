; STA ($FF),Y - the pointer's high byte must come from $00, not $0100.
; ($FF)=$00, ($00)=$80 -> $8000; with the bug the store lands at $9000.
$4000   LDAI #$00
        STAZ $FF
        LDAI #$80
        STAZ $00
        LDAI #$90
        STAA $0100
        LDYI #$00
        LDAI #$5A
        STAIY $FF
        BRK
