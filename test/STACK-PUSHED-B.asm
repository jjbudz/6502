; PHP pushes P with B (bit 4) and bit 5 set, as on the 6502. A handler
; checks the pushed copy this way, through TSX and page 1. Stores the
; pushed B and bit 5 at $8000: expect $30.
        .ORG $4000
        CLC
        PHP
        TSX
        LDA $0101,X     ; the byte PHP just pushed
        AND #$30
        STA $8000
        BRK
