; Legacy (suffixed) and standard mnemonics may be mixed in one file.
$4000   LDAI #$05        ; legacy immediate
        STA $50          ; standard zero page
        LDAZ $50         ; legacy zero page
        CLC
        ADC #$01         ; standard immediate
        STAA $8000       ; legacy absolute
        BRK
