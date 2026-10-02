; Complex register transfer and stack operations test
; Tests TAX, TAY, TXA, TYA, TXS, TSX, DEX, DEY, INX, INY
; Expected result: $40 = $33 after series of transfers and modifications
; Tests register preservation and stack pointer operations
$4000 	LDAI #$35

        TAX
        DEX
        DEX
        INX
        TXA

        TAY
        DEY
        DEY
        INY
        TYA

        TAX
        LDAI #$20
        TXS
        LDXI #$10
        TSX
        TXA

        STAZ $40
       
