        ;;  expected result: $022A = 0x55
$4000   LDAI #85
        LDXI #42
        LDYI #115
        STAZ $81
        LDAI #$01
        STAZ $61
        LDAI #$7E
        LDAZ $81
        STAA $0910
        LDAI #$7E
        LDAA $0910
        STAZX $56
        LDAI #$7E
        LDAZX $56
        STYZ $60
        STAIY $60
        LDAI #$7E
        LDAIY $60
        STAX $07ff
        LDAI #$7E
        LDAX $07ff
        STAY $07ff
        LDAI #$7E
        LDAY $07ff
        STAZX $36
        LDAI #$7E
        LDAZX $36
        STXZ $50
        LDXZ $60
        LDYZ $50
        STXA $0913
        LDXI #$22
        LDXA $0913
        STYA $0914
        LDYI #$99
        LDYA $0914
        STYZX $2D
        STXZY $77
        LDYI #$99
        LDYZX $2D
        LDXI #$22
        LDXZY $77
        LDYI #$99
        LDYX $08A0
        LDXI #$22
        LDXY $08A1
        STAX $0200

        ;;  CHECK test00:
        LDAA $022A
        CMPA $0200
        BEQ test00pass
        JMP theend
test00pass LDAI #$FE
           STAA $0210
theend BRK
        
        
