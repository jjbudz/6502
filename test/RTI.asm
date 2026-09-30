; Test RTI (Return from Interrupt)
; Tests: RTI
; Simulates interrupt context by manually pushing return address
; Push return address $400B (where execution should resume)
; Expected: RTI pops status and return address from stack
; Result stored at $8000 should be $01 (success)
$4000   LDAI #$40
        PHA
        LDAI #$0b
        PHA
        SEC
        PHP
        CLI
        SEI
        RTI
        LDAI #$01
        STAA $8000
        BRK
