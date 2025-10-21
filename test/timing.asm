;; Test timing simulation - deterministic loop to validate ticker wall-clock timing
;; Loop uses X as iteration counter.
;; Cycle math:
;;   LDX #imm = 2
;;   Per-iteration (taken BNE): NOP(2) + DEX(2) + BNE taken(3) = 7
;;   Last iteration (BNE not taken): NOP(2) + DEX(2) + BNE not-taken(2) = 6
;; Total cycles = 2 + (N-1)*7 + 6 = 7*N + 1
;; With N = $0F (15), total_cycles = 7*15 + 1 = 106
        LDX #$0F         ; N = 15 iterations
loop:
        NOP
        DEX
        BNE loop
        STA $9000        ; write sentinel to memory
        BRK
