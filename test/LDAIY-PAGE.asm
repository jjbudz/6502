; Test LDA Indirect Indexed - Page boundary crossing in effective address
; Tests: LDA ($nn),Y when adding Y crosses page boundary
; Expected: Correctly reads across page boundary
; Result stored at $8000 should be $66
$4000   LDAI #$FF     ; Low byte of base address
        STAZ $80
        LDAI #$01     ; High byte ($01FF)
        STAZ $81
        LDAI #$66
        STAA $0200    ; Store test value at $0200
        LDYI #$01     ; Y = 1, so $01FF + $01 = $0200 (crosses page)
        LDAIY $80     ; Load from ($80),Y = $01FF + $01 = $0200
        STAA $8000
        BRK
