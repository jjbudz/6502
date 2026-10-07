; Test Zero Flag - SBC Producing Zero
; Tests: $42 - $42 = $00 sets Z=1, C=1 (no borrow)
; Result stored at $8000 should be $01 (success)
$4000   SEC           ; No borrow
        LDAI #$42
        SBCI #$42
        BEQ ok1       ; Z must be set
        BRK
ok1     BCS pass      ; C must remain set
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
