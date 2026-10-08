; A label that is a prefix of another label must not be confused with it.
; The old assembler redefined LOOP when it saw it as the operand on the
; LOOP2 line (it matched LOOP against the start of the line), so the branch
; went to the wrong place. Counts X down from 3, adding 1 to A each time.
$4000   LDXI #$03
        LDAI #$00
LOOP    CLC
        ADCI #$01
        DEX
LOOP2   BNE LOOP
        STAA $8000
        BRK
