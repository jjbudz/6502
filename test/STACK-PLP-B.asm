; PLP discards B and bit 5: they have no storage in P. Pulling $FF must not
; set the emulator's break flag, which would halt the run right after the
; PLP. Stores the other six flags (via PHP/PLA) at $8000: expect $CF.
        .ORG $4000
        LDA #$FF
        PHA
        PLP             ; N V D I Z C set; B and bit 5 discarded
        PHP
        PLA
        AND #$CF
        STA $8000
        BRK
