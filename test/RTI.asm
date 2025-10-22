; RTI test - simulate interrupt context by manually pushing return address
; Push return address $400B (where execution should resume)
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
