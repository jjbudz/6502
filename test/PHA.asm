; Test PHA (Push Accumulator) and PLA (Pull Accumulator)
; Tests: PHA, PLA
; Expected: Push $FF to stack, load 0, then pull $FF back from stack
; Result stored at $1000 should be $FF
$4000   LDAI #$ff
	    PHA
        LDAI #000
        PLA
        STAA $1000
	    BRK
