; Test PHP (Push Processor Status) and PLP (Pull Processor Status)
; Tests: PHP, PLP
; Expected: Push status with carry set, clear carry, pull status back
; Carry should be set again after PLP
; Result stored at $8000 should be $01 (success)
$4000   SEC
        PHP
        CLC
        PLP
        BCS pass
        LDAI #$00
        STAA $8000
        BRK
pass    LDAI #$01
        STAA $8000
        BRK
