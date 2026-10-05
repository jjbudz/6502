; Test LDA Absolute,X - Page boundary crossing
; Tests: LDA $addr,X when X causes page crossing
; Expected: Reads from $0200 when base is $0100 and X is $FF
; Result stored at $8000 should be $42
$4000   LDAI #$42
        STAA $0200    ; Store test value at target
        LDXI #$FF     ; Set X to cause page crossing
        LDAX $0101    ; $0101 + $FF = $0200 (crosses page)
        STAA $8000
        BRK
