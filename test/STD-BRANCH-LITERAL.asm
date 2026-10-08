; Standard syntax: a branch to a literal address is encoded as a relative
; offset. The old assembler emitted it as a 16-bit absolute operand.
$4000   LDA #$00
        BEQ $400A        ; taken: skip the failure path
$4004   LDA #$FF
        STA $8000
        BRK
$400A   LDA #$01
        STA $8000
        BRK
