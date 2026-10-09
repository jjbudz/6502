;; Test timing simulation - deterministic loop to validate ticker wall-clock timing
;; Loop uses X as iteration counter.
;; Cycle math:
;;   LDXI #imm = 2
;;   Per-iteration (taken BNE): NOP(2) + DEX(2) + BNE taken(3) = 7
;;   Last iteration (BNE not taken): NOP(2) + DEX(2) + BNE not-taken(2) = 6
;;   STAA abs = 4, BRK = 7
;; Total cycles = 2 + (N-1)*7 + 6 + 4 + 7 = 7*N + 12
;; With N = $0F (15), total_cycles = 7*15 + 12 = 117
$4000   LDXI #$0F         ; N = 15 iterations
loop    NOP
        DEX
        BNE loop
        STAA $9000        ; write sentinel to memory
        BRK
