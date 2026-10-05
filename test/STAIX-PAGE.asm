; Test STA Indexed Indirect - Page boundary in pointer
; Tests: STA ($nn,X) when pointer calculation crosses zero page
; Expected: Correct pointer fetch even at zero page boundary
; Result: Value stored correctly via indirect addressing
$4000   LDAI #$50     ; Low byte of target address
        STAZ $FE      ; Store at $FE
        LDAI #$80     ; High byte of target address
        STAZ $FF      ; Store at $FF (page boundary)
        LDXI #$FE     ; X = $FE, so ($00,X) reads from $FE-$FF
        LDAI #$AA
        STAIX $00     ; Store via ($00,X) = ($FE) = $8050
        LDAA $8050    ; Verify it was stored
        STAA $8000
        BRK
