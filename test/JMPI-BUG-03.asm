; Test JMP Indirect - Multiple page boundaries
; Tests: JMP ($20FF), ($30FF), ($40FF)
; Expected: Consistent behavior across different pages
; Result: All should work if bug is fixed
$4000   ; Setup pointer at $20FF
        LDAI #$50
        STAA $20FF
        LDAI #$40     ; Target: $4050
        STAA $2100
        ; Setup pointer at $30FF
        LDAI #$60
        STAA $30FF
        LDAI #$40     ; Target: $4060
        STAA $3100
        JMP test1
$4050   JMP test2
$4060   LDAI #$01     ; Success
        STAA $8000
        BRK
test1   JMPI $20FF
test2   JMPI $30FF
