; Test LDA Absolute,Y - Page boundary crossing
; Tests: LDA $addr,Y when Y causes page crossing
; Expected: Reads from $0300 when base is $0201 and Y is $FF
; Result stored at $8000 should be $55
$4000   LDAI #$55
        STAA $0300    ; Store test value at target
        LDYI #$FF     ; Set Y to cause page crossing
        LDAY $0201    ; $0201 + $FF = $0300 (crosses page)
        STAA $8000
        BRK
