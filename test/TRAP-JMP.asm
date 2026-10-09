; A JMP to itself ends the run. Stores $01 at $8000 first; the test also
; checks that the run stops at the loop ($4005).
        .ORG $4000
        LDA #$01
        STA $8000
done    JMP done
