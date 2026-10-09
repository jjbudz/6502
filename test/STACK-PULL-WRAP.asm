; Pulling with SP=$FF wraps SP to $00 and reads $0100, the bottom of
; page 1. Stores the pulled value at $8000: expect $77.
        .ORG $4000
        LDA #$77
        STA $0100
        PLA             ; SP $FF -> $00, reads $0100
        STA $8000
        BRK
