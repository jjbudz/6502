; Test STA Absolute,X - Page boundary crossing
; Tests: STA $addr,X when X causes page crossing
; Expected: Stores to $0300 when base is $0201 and X is $FF
; Verification reads from $0300 should get $77
$4000   LDXI #$FF     ; Set X to cause page crossing
        LDAI #$77
        STAX $0201    ; $0201 + $FF = $0300 (crosses page)
        LDAA $0300    ; Verify it was stored
        STAA $8000
        BRK
