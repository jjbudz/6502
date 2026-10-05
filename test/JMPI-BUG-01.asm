; Test JMP Indirect - Page boundary bug (6502 hardware bug)
; Tests: JMP ($xxFF) page boundary behavior
; The 6502 reads from $xxFF and $xx00 instead of $xxFF and $(xx+1)00
; Expected: If emulating bug, jumps to wrong address; if fixed, jumps correctly
; This test checks for CORRECT behavior (bug fixed)
$4000   LDAI #$20     ; Low byte of correct target
        STAA $10FF    ; Store at page boundary
        LDAI #$40     ; High byte -> target $4020
        STAA $1100    ; Store at next page
        LDAI #$AD     ; Low byte of wrong target (if bug exists)
        STAA $1000    ; Store at $1000
        LDAI #$DE     ; High byte -> would be $DEAD if bug exists
        STAA $1001
        JMP test
$4020   LDAI #$01     ; Correct destination (bug fixed)
        STAA $8000
        BRK
$4030   LDAI #$FF     ; Wrong destination (bug exists)
        STAA $8000
        BRK
test    JMPI $10FF    ; Jump via pointer at page boundary
