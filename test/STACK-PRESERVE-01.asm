; Test PHP/PLP Flag Preservation
; Tests: All flags preserved through PHP/PLP including B flag behavior
; Expected: Flags restored correctly after PHP/PLP sequence
$4000   CLC           ; Clear carry
        CLV           ; Clear overflow
        SEC           ; Set carry
        LDAI #$80     ; Load negative value
        PHP           ; Push flags
        CLC           ; Clear carry
        PLP           ; Pull flags - carry should be set again
        LDAI #$00
        ADCI #$00     ; Add with carry - should give $01
        STAA $8000    ; Should be $01 if carry was restored
        BRK
