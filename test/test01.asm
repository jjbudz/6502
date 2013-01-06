        ;;  expected result: $A9 = 0xAA
        ;;  imm
$4000   LDAI #85
        ANDI #83
        ORAI #56
        EORI #17

        ;;  zpg
        STAZ $99
        LDAI #185
        STAZ $10
        LDAI #231
        STAZ $11
        LDAI #57
        STAZ $12
        LDAZ $99
        ANDZ $10
        ORAZ $11
        EORZ $12

        ;;  zpx
        LDXI #16
        STAZ $99
        LDAI #188
        STAZ $20
        LDAI #49
        STAZ $21
        LDAI #23
        STAZ $22
        LDAZ $99
        ANDZX $10
        ORAZX $11
        EORZX $12

        ;;  abs
        STAZ $99
        LDAI #111
        STAA $0110
        LDAI #60
        STAA $0111
        LDAI #39
        STAA $0112
        LDAZ $99
        ANDA $0110
        ORAA $0111
        EORA $0112

        ;;  abx
        STAZ $99
        LDAI #138
        STAA $0120
        LDAI #71
        STAA $0121
        LDAI #143
        STAA $0122
        LDAZ $99
        ANDX $0110
        ORAX $0111
        EORX $0112

        ;;  aby
        LDYI #32
        STAZ $99
        LDAI #115
        STAA $0130
        LDAI #42
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
        LDAI #$01
        STAZ $31
        LDAI #113
        STAZ $32
        LDAI #$01
        STAZ $33
        LDAI #114
        STAZ $34
        LDAI #01
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
        LDAI #96
        STAZ $40
        LDAI #$01
        STAZ $41
        LDAI #97
        STAZ $42
        LDAI #$01
        STAZ $43
        LDAI #98
        STAZ $44
        LDAI #$01
        STAZ $45
        LDAI #55
        STAA $0250
        LDAI #35
        STAA $0251
        LDAI #157
        STAA $0252
        LDAZ $99
        LDYI #F0
        ANDIY $40
        ORAIY $42
        EORIY $44

        STAZ $A9

;;  CHECK test01
        LDAZ $A9
        CMPA $0201
        BEQ theend
        LDAI #$01
        STAA $0210
        JMP theend

theend  BRK
        
