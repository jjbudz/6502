; Test SBC Borrow Flag
; Tests: SBC correctly handles borrow (inverted carry) flag
; Expected: Borrow propagates correctly through subtraction
$4000   SEC           ; Set carry (no borrow)
        LDAI #$50
        SBCI #$30     ; $50 - $30 = $20, carry stays set
        BCS $400C     ; Should branch (no borrow)
        BRK           ; Shouldn't reach
$400C   CLC           ; Clear carry (borrow)
        LDAI #$50
        SBCI #$30     ; $50 - $30 - 1 = $1F with borrow
        CMPI #$1F
        BEQ $4018     ; Should branch
        BRK           ; Shouldn't reach
$4018   LDAI #$01
        STAA $8000    ; Success
        BRK
