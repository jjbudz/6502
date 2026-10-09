; PLA sets N and Z from the pulled value, like any load into A. Each pull
; follows an LDA that leaves the opposite flags, so a PLA that skipped them
; would branch to fail. Stores $01 at $8000 on success, $00 on failure.
        .ORG $4000
        LDA #$00
        PHA
        LDA #$01        ; Z = 0, N = 0
        PLA             ; $00: Z = 1
        BNE fail
        LDA #$80
        PHA
        LDA #$01        ; Z = 0, N = 0
        PLA             ; $80: N = 1
        BPL fail
        LDA #$01
        PHA
        LDA #$80        ; N = 1
        PLA             ; $01: N = 0, Z = 0
        BMI fail
        BEQ fail
        LDA #$01
        STA $8000
        BRK
fail    LDA #$00
        STA $8000
        BRK
