; Driven by the debugger's IRQ and NMI commands (see test-DEBUG-IRQ): after
; CLI (2 cycles), "irq" then "step" takes the IRQ (7 cycles) to $4010; after
; RTI (6), "nmi" then "step" takes the NMI (7) to $4011.
        .ORG $4000
        CLI
loop    JMP loop

        .ORG $4010
irq     RTI
nmi     RTI

        .ORG $FFFA
        .WORD nmi
        .WORD $4000
        .WORD irq
