; Test STA Absolute,Y - Page boundary crossing
; Tests: STA $addr,Y when Y causes page crossing
; Expected: Stores to $0400 when base is $0301 and Y is $FF
; Verification reads from $0400 should get $88
$4000   LDYI #$FF     ; Set Y to cause page crossing
        LDAI #$88
        STAY $0301    ; $0301 + $FF = $0400 (crosses page)
        LDAA $0400    ; Verify it was stored
        STAA $8000
        BRK
