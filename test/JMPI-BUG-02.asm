; Test JMP Indirect - Non-boundary case (should always work)
; Tests: JMP ($xxFE) - not at page boundary
; Expected: Always works correctly regardless of bug
; Result: Should jump to correct target
$4000   LDAI #$30
        STAA $10FE
        LDAI #$40     ; Target: $4030
        STAA $10FF
        JMP test
$4030   LDAI #$01     ; Correct destination
        STAA $8000
        BRK
test    JMPI $10FE    ; Not at boundary, should work
