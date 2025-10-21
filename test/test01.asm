        ;;  expected result: $A9 = 0xAA
        ;;  imm
$4000   LDAI #085
        ANDI #083
        ORAI #056
        EORI #017

        ;;  zpg
        STAZ $99
        LDAI #185
        STAZ $10
        LDAI #231
        STAZ $11
        LDAI #057
        STAZ $12
        LDAZ $99
        ANDZ $10
        ORAZ $11
        EORZ $12

        ;;  zpx
        LDXI #016
        STAZ $99
        LDAI #188
        STAZ $20
        LDAI #049
        STAZ $21
        LDAI #023
        STAZ $22
        LDAZ $99
        ANDZX $10
        ORAZX $11
        EORZX $12

        ;;  abs
        STAZ $99
        LDAI #111
        STAA $0110
        LDAI #060
        STAA $0111
        LDAI #039
        STAA $0112
        LDAZ $99
        ANDA $0110
        ORAA $0111
        EORA $0112

        ;;  abx
        STAZ $99
        LDAI #138
        STAA $0120
        LDAI #071
        STAA $0121
        LDAI #143
        STAA $0122
        LDAZ $99
        ANDX $0110
        ORAX $0111
        EORX $0112

        ;;  aby
        LDYI #032
        STAZ $99
        LDAI #115
        STAA $0130
        LDAI #042
        STAA $0131
        LDAI #241
        STAA $0132
        LDAZ $99
        ANDY $0110
        ORAY $0111
        EORY $0112

        ;;  idx
        STAZ $99
        LDAI #112
        STAZ $30
        LDAI #001
        STAZ $31
        LDAI #113
        STAZ $32
        LDAI #001
        STAZ $33
        LDAI #114
        STAZ $34
        LDAI #001
        STAZ $35
        LDAI #197
        STAA $0170
        LDAI #124
        STAA $0171
        LDAI #161
        STAA $0172
        LDAZ $99
        ANDIX $20
        ORAIX $22
        EORIX $24

        ;;  idy
        STAZ $99
        LDAI #096
        STAZ $40
        LDAI #001
        STAZ $41
        LDAI #097
        STAZ $42
        LDAI #001
        STAZ $43
        LDAI #098
        STAZ $44
        LDAI #001
        STAZ $45
        LDAI #055
        STAA $0250
        LDAI #035
        STAA $0251
        LDAI #157
        STAA $0252
        LDAZ $99
        LDYI #$F0
        ANDIY $40
        ORAIY $42
        EORIY $44

        STAZ $A9

;;  CHECK test01
        LDAZ $A9
        CMPI #$AA
        BEQ theend
        LDAI #001
        STAA $0210
        JMP theend

theend  BRK
        
