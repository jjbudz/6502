; A taken branch to itself ends the run, but a branch to itself that is not
; taken falls through. Stores $01 at $8000 only if the untaken one fell
; through; the test also checks that the run stops at the taken one ($4008).
        .ORG $4000
        SEC
skip    BCC skip            ; not taken: continues
        LDA #$01
        STA $8000
done    BCS done            ; taken: ends the run
