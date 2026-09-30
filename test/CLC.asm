; Test CLC (Clear Carry Flag)
; Tests: CLC
; Expected: Carry flag cleared after SEC, BCC should succeed
; Result stored at $8000 should be $01 (success)
$4000   SEC
        BCC fail1
        CLC
        BCS fail2
        JMP pass
fail1   LDAI  #$FF
        STAA  $8000
        BRK
fail2   LDAI  #$FE
        STAA  $8000
        BRK
pass    LDAI  #001
        STAA  $8000
        BRK
