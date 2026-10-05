; Test JSR with Stack Near Bottom
; Tests: JSR correctly pushes return address when SP is near $00
; Expected: Return address pushed to stack, RTS returns correctly
$4000   LDXI #$02     ; Set stack pointer near bottom
        TXS
        JSR $4010     ; Call subroutine
        LDAI #$42     ; Should execute after RTS
        STAA $8000
        BRK
$4010   RTS           ; Return from subroutine
