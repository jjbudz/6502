
include ../include.mk

# Path to the emulator binary (in parent directory's bin directory)
EMU = ../$(BINDIR)/6502

# Individual test targets
.PHONY: test test-ADCA test-ADCI test-ADCIX test-ADCIY test-ADCX test-ADCY test-ADCZ test-ADCZX test-ANDA test-ANDI test-ANDIX test-ANDIY test-ANDX test-ANDY test-ANDZ test-ANDZX test-ASL test-ASLA test-ASLX test-ASLZ test-ASLZX test-BCC test-BCS test-BEQ test-BIT test-BITZ test-BMI test-BNE test-BPL test-BRK test-BVC test-BVS test-CLC test-CLD test-CLI test-CLV test-CMPA test-CMPI test-CMPIX test-CMPIY test-CMPX test-CMPY test-CMPZ test-CMPZX test-CPXA test-CPXI test-CPXZ test-CPYA test-CPYI test-CPYZ test-DECA test-DECX test-DECZ test-DECZX test-DEX test-DEY test-EORA test-EORI test-EORIX test-EORIY test-EORX test-EORY test-EORZ test-EORZX test-INCA test-INCX test-INCZ test-INCZX test-INX test-INY test-JMP test-JMPI test-JSR test-LDAA test-LDAI1 test-LDAI2 test-LDAI3 test-LDAIX test-LDAIY test-LDAX test-LDAY test-LDAZ test-LDAZX test-LDXA test-LDXY test-LDXZ test-LDXZY test-LDYA test-LDYX test-LDYZ test-LDYZX test-LSR test-LSRA test-LSRX test-LSRZ test-LSRZX test-NOP test-ORAA test-ORAI test-ORAIX test-ORAIY test-ORAX test-ORAY test-ORAZ test-ORAZX test-PHA test-PHP test-PLA test-PLP test-ROL test-ROLA test-ROLX test-ROLZ test-ROLZX test-ROR test-RORA test-RORX test-RORZ test-RORZX test-RTI test-RTS test-SBCA test-SBCI test-SBCIX test-SBCIY test-SBCX test-SBCY test-SBCZ test-SBCZX test-SEC test-SED test-SEI test-STAA test-STAIX test-STAIY test-STAX test-STAY test-STAZ test-STAZX test-STXA test-STXZ test-STXZY test-STYA test-STYZ test-STYZX test-TAX test-TAY test-TSX test-TXA test-TXS test-TYA test-test00 test-test01 test-test05 test-timing test-BCD-ADC-01 test-BCD-ADC-02 test-BCD-ADC-03 test-BCD-ADC-04 test-BCD-ADC-05 test-BCD-ADC-06 test-BCD-SBC-01 test-BCD-SBC-02 test-BCD-SBC-03 test-LDAX-PAGE test-LDAY-PAGE test-STAIX-PAGE test-LDAIY-PAGE test-STAX-PAGE test-STAY-PAGE test-LDAZX-WRAP test-LDAZY-WRAP test-STAZX-WRAP test-STXZY-WRAP test-STYZX-WRAP test-ANDIX-WRAP test-JMPI-BUG-01 test-JMPI-BUG-02 test-JMPI-BUG-03 test-STACK-WRAP-01 test-STACK-WRAP-02 test-STACK-JSR-01 test-STACK-PRESERVE-01 test-STACK-FULL-01 test-FLAG-ADC-NVZ test-FLAG-SBC-BORROW test-FLAG-OVERFLOW-01 test-FLAG-OVERFLOW-02 test-FLAG-ZERO-ADC test-FLAG-ZERO-SBC test-BOUNDARY-INC-FF test-BOUNDARY-DEC-00 test-BOUNDARY-ADC-7F test-BOUNDARY-SBC-80 test-BOUNDARY-ASL-80 test-BOUNDARY-LSR-01 test-BOUNDARY-ROL-FF test-BCD-SBC-04 test-BCD-ADC-ZP test-BCD-FLAG-NV-01 test-BCD-FLAG-NV-02 test-BCD-FLAG-Z test-BCD-SBC-FLAGS test-OPCODE-ADC-61 test-OPCODE-ADC-75 test-OPCODE-AND-21 test-OPCODE-AND-35 test-OPCODE-SBC-E1 test-OPCODE-SBC-F5 test-ZPWRAP-LDAIY test-ZPWRAP-LDAIX test-ZPWRAP-ADCIY test-ZPWRAP-SBCIX test-ZPWRAP-STAIY test-ZPWRAP-STAIX test-ADDRWRAP-LDAX test-ADDRWRAP-LDAY test-ADDRWRAP-STAX test-ADDRWRAP-INCX test-ADDRWRAP-LDAIY test-ADDRWRAP-STAIY test-DATA-PREFIX

# Run all tests using the test runner script
test:
	@echo "========================================="
	@echo "Running 6502 Emulator Test Suite"
	@echo "========================================="
	@bash run_tests.sh

test-ADCA:
	@echo "Test ADCA"
	$(EMU) -c ADCA.asm -r 4000 -a 8000:80

test-ADCI:
	@echo "Test ADCI"
	$(EMU) -c ADCI.asm -r 4000 -a 8000:ff

test-ADCIX:
	@echo "Test ADCIX"
	$(EMU) -c ADCIX.asm -r 4000 -a 8000:80

test-ADCIY:
	@echo "Test ADCIY"
	$(EMU) -c ADCIY.asm -r 4000 -a 8000:80

test-ADCX:
	@echo "Test ADCX"
	$(EMU) -c ADCX.asm -r 4000 -a 8000:80

test-ADCY:
	@echo "Test ADCY"
	$(EMU) -c ADCY.asm -r 4000 -a 8000:80

test-ADCZ:
	@echo "Test ADCZ"
	$(EMU) -c ADCZ.asm -r 4000 -a 8000:80

test-ADCZX:
	@echo "Test ADCZX"
	$(EMU) -c ADCZX.asm -r 4000 -a 8000:80

test-ANDA:
	@echo "Test ANDA"
	$(EMU) -c ANDA.asm -r 4000 -a 8000:55

test-ANDI:
	@echo "Test ANDI"
	$(EMU) -c ANDI.asm -r 4000 -a 8000:55

test-ANDIX:
	@echo "Test ANDIX"
	$(EMU) -c ANDIX.asm -r 4000 -a 8000:55

test-ANDIY:
	@echo "Test ANDIY"
	$(EMU) -c ANDIY.asm -r 4000 -a 8000:55

test-ANDX:
	@echo "Test ANDX"
	$(EMU) -c ANDX.asm -r 4000 -a 8000:55

test-ANDY:
	@echo "Test ANDY"
	$(EMU) -c ANDY.asm -r 4000 -a 8000:55

test-ANDZ:
	@echo "Test ANDZ"
	$(EMU) -c ANDZ.asm -r 4000 -a 8000:55

test-ANDZX:
	@echo "Test ANDZX"
	$(EMU) -c ANDZX.asm -r 4000 -a 8000:55

test-ASL:
	@echo "Test ASL"
	$(EMU) -c ASL.asm -r 4000 -a 8000:aa

test-ASLA:
	@echo "Test ASLA"
	$(EMU) -c ASLA.asm -r 4000 -a 8000:aa

test-ASLX:
	@echo "Test ASLX"
	$(EMU) -c ASLX.asm -r 4000 -a 8000:aa

test-ASLZ:
	@echo "Test ASLZ"
	$(EMU) -c ASLZ.asm -r 4000 -a 8000:aa

test-ASLZX:
	@echo "Test ASLZX"
	$(EMU) -c ASLZX.asm -r 4000 -a 8000:aa

test-BCC:
	@echo "Test BCC"
	$(EMU) -c BCC.asm -r 4000 -a 8000:01

test-BCS:
	@echo "Test BCS"
	$(EMU) -c BCS.asm -r 4000 -a 8000:01

test-BEQ:
	@echo "Test BEQ"
	$(EMU) -c BEQ.asm -r 4000 -a 8000:01

test-BIT:
	@echo "Test BIT"
	$(EMU) -c BIT.asm -r 4000 -a 8000:01

test-BITZ:
	@echo "Test BITZ"
	$(EMU) -c BITZ.asm -r 4000 -a 8000:01

test-BMI:
	@echo "Test BMI"
	$(EMU) -c BMI.asm -r 4000 -a 8000:01

test-BNE:
	@echo "Test BNE"
	$(EMU) -c BNE.asm -r 4000 -a 8000:01

test-BPL:
	@echo "Test BPL"
	$(EMU) -c BPL.asm -r 4000 -a 8000:01

test-BRK:
	@echo "Test BRK"
	$(EMU) -c BRK.asm -r 4000 -a 8000:01

test-BVC:
	@echo "Test BVC"
	$(EMU) -c BVC.asm -r 4000 -a 8000:01

test-BVS:
	@echo "Test BVS"
	$(EMU) -c BVS.asm -r 4000 -a 8000:01

test-CLC:
	@echo "Test CLC"
	$(EMU) -c CLC.asm -r 4000 -a 8000:01

test-CLD:
	@echo "Test CLD"
	$(EMU) -c CLD.asm -r 4000 -a 8000:01

test-CLI:
	@echo "Test CLI"
	$(EMU) -c CLI.asm -r 4000 -a 8000:01

test-CLV:
	@echo "Test CLV"
	$(EMU) -c CLV.asm -r 4000 -a 8000:01

test-CMPA:
	@echo "Test CMPA"
	$(EMU) -c CMPA.asm -r 4000 -a 8000:01

test-CMPI:
	@echo "Test CMPI"
	$(EMU) -c CMPI.asm -r 4000 -a 8000:01

test-CMPIX:
	@echo "Test CMPIX"
	$(EMU) -c CMPIX.asm -r 4000 -a 8000:01

test-CMPIY:
	@echo "Test CMPIY"
	$(EMU) -c CMPIY.asm -r 4000 -a 8000:01

test-CMPX:
	@echo "Test CMPX"
	$(EMU) -c CMPX.asm -r 4000 -a 8000:01

test-CMPY:
	@echo "Test CMPY"
	$(EMU) -c CMPY.asm -r 4000 -a 8000:01

test-CMPZ:
	@echo "Test CMPZ"
	$(EMU) -c CMPZ.asm -r 4000 -a 8000:01

test-CMPZX:
	@echo "Test CMPZX"
	$(EMU) -c CMPZX.asm -r 4000 -a 8000:01

test-CPXA:
	@echo "Test CPXA"
	$(EMU) -c CPXA.asm -r 4000 -a 8000:01

test-CPXI:
	@echo "Test CPXI"
	$(EMU) -c CPXI.asm -r 4000 -a 8000:01

test-CPXZ:
	@echo "Test CPXZ"
	$(EMU) -c CPXZ.asm -r 4000 -a 8000:01

test-CPYA:
	@echo "Test CPYA"
	$(EMU) -c CPYA.asm -r 4000 -a 8000:01

test-CPYI:
	@echo "Test CPYI"
	$(EMU) -c CPYI.asm -r 4000 -a 8000:01

test-CPYZ:
	@echo "Test CPYZ"
	$(EMU) -c CPYZ.asm -r 4000 -a 8000:01

test-DECA:
	@echo "Test DECA"
	$(EMU) -c DECA.asm -r 4000 -a 8000:1f

test-DECX:
	@echo "Test DECX"
	$(EMU) -c DECX.asm -r 4000 -a 8000:3f

test-DECZ:
	@echo "Test DECZ"
	$(EMU) -c DECZ.asm -r 4000 -a 8000:0f

test-DECZX:
	@echo "Test DECZX"
	$(EMU) -c DECZX.asm -r 4000 -a 8000:2f

test-DEX:
	@echo "Test DEX"
	$(EMU) -c DEX.asm -r 4000 -a 8000:01

test-DEY:
	@echo "Test DEY"
	$(EMU) -c DEY.asm -r 4000 -a 8000:01

test-EORA:
	@echo "Test EORA"
	$(EMU) -c EORA.asm -r 4000 -a 8000:01

test-EORI:
	@echo "Test EORI"
	$(EMU) -c EORI.asm -r 4000 -a 8000:01

test-EORIX:
	@echo "Test EORIX"
	$(EMU) -c EORIX.asm -r 4000 -a 8000:ff

test-EORIY:
	@echo "Test EORIY"
	$(EMU) -c EORIY.asm -r 4000 -a 8000:ff

test-EORX:
	@echo "Test EORX"
	$(EMU) -c EORX.asm -r 4000 -a 8000:01

test-EORY:
	@echo "Test EORY"
	$(EMU) -c EORY.asm -r 4000 -a 8000:01

test-EORZ:
	@echo "Test EORZ"
	$(EMU) -c EORZ.asm -r 4000 -a 8000:01

test-EORZX:
	@echo "Test EORZX"
	$(EMU) -c EORZX.asm -r 4000 -a 8000:01

test-INCA:
	@echo "Test INCA"
	$(EMU) -c INCA.asm -r 4000 -a 8000:21

test-INCX:
	@echo "Test INCX"
	$(EMU) -c INCX.asm -r 4000 -a 8000:41

test-INCZ:
	@echo "Test INCZ"
	$(EMU) -c INCZ.asm -r 4000 -a 8000:11

test-INCZX:
	@echo "Test INCZX"
	$(EMU) -c INCZX.asm -r 4000 -a 8000:31

test-INX:
	@echo "Test INX"
	$(EMU) -c INX.asm -r 4000 -a 8000:01

test-INY:
	@echo "Test INY"
	$(EMU) -c INY.asm -r 4000 -a 8000:01

test-JMP:
	@echo "Test JMP"
	$(EMU) -c JMP.asm -r 4000 -a 8000:01

test-JMPI:
	@echo "Test JMPI"
	$(EMU) -c JMPI.asm -r 4000 -a 8000:01

test-JSR:
	@echo "Test JSR"
	$(EMU) -c JSR.asm -r 4000 -a 8000:01

test-LDAA:
	@echo "Test LDAA"
	$(EMU) -c LDAA.asm -r 4000 -a 8000:7f

test-LDAI1:
	@echo "Test LDA1"
	$(EMU) -c LDAI1.asm -r 4000 -a 8000:7f

test-LDAI2:
	@echo "Test LDA2"
	$(EMU) -c LDAI2.asm -r 4000 -a 8000:7f

test-LDAI3:
	@echo "Test LDA3"
	$(EMU) -c LDAI3.asm -r 4000 -a 8000:00

test-LDAIX:
	@echo "Test LDAIX"
	$(EMU) -c LDAIX.asm -r 4000 -a 8000:7f

test-LDAIY:
	@echo "Test LDAIY"
	$(EMU) -c LDAIY.asm -r 4000 -a 8000:7f

test-LDAX:
	@echo "Test LDAX"
	$(EMU) -c LDAX.asm -r 4000 -a 8000:7f

test-LDAY:
	@echo "Test LDAY"
	$(EMU) -c LDAY.asm -r 4000 -a 8000:7f

test-LDAZ:
	@echo "Test LDAZ"
	$(EMU) -c LDAZ.asm -r 4000 -a 8000:7f

test-LDAZX:
	@echo "Test LDAZX"
	$(EMU) -c LDAZX.asm -r 4000 -a 8000:7f

test-LDXA:
	@echo "Test LDXA"
	$(EMU) -c LDXA.asm -r 4000 -a 8000:7e

test-LDXY:
	@echo "Test LDXY"
	$(EMU) -c LDXY.asm -r 4000 -a 8000:7e

test-LDXZ:
	@echo "Test LDXZ"
	$(EMU) -c LDXZ.asm -r 4000 -a 8000:7e

test-LDXZY:
	@echo "Test LDXZY"
	$(EMU) -c LDXZY.asm -r 4000 -a 8000:7e

test-LDYA:
	@echo "Test LDYA"
	$(EMU) -c LDYA.asm -r 4000 -a 8000:5a

test-LDYX:
	@echo "Test LDYX"
	$(EMU) -c LDYX.asm -r 4000 -a 8000:5a

test-LDYZ:
	@echo "Test LDYZ"
	$(EMU) -c LDYZ.asm -r 4000 -a 8000:5a

test-LDYZX:
	@echo "Test LDYZX"
	$(EMU) -c LDYZX.asm -r 4000 -a 8000:5a

test-LSR:
	@echo "Test LSR"
	$(EMU) -c LSR.asm -r 4000 -a 8000:55

test-LSRA:
	@echo "Test LSRA"
	$(EMU) -c LSRA.asm -r 4000 -a 8000:55

test-LSRX:
	@echo "Test LSRX"
	$(EMU) -c LSRX.asm -r 4000 -a 8000:55

test-LSRZ:
	@echo "Test LSRZ"
	$(EMU) -c LSRZ.asm -r 4000 -a 8000:55

test-LSRZX:
	@echo "Test LSRZX"
	$(EMU) -c LSRZX.asm -r 4000 -a 8000:55

test-NOP:
	@echo "Test NOP"
	$(EMU) -c NOP.asm -r 4000 -a 8000:01

test-ORAA:
	@echo "Test ORAA"
	$(EMU) -c ORAA.asm -r 4000 -a 8000:5f

test-ORAI:
	@echo "Test ORAI"
	$(EMU) -c ORAI.asm -r 4000 -a 8000:5f

test-ORAIX:
	@echo "Test ORAIX"
	$(EMU) -c ORAIX.asm -r 4000 -a 8000:5f

test-ORAIY:
	@echo "Test ORAIY"
	$(EMU) -c ORAIY.asm -r 4000 -a 8000:5f

test-ORAX:
	@echo "Test ORAX"
	$(EMU) -c ORAX.asm -r 4000 -a 8000:5f

test-ORAY:
	@echo "Test ORAY"
	$(EMU) -c ORAY.asm -r 4000 -a 8000:5f

test-ORAZ:
	@echo "Test ORAZ"
	$(EMU) -c ORAZ.asm -r 4000 -a 8000:5f

test-ORAZX:
	@echo "Test ORAZX"
	$(EMU) -c ORAZX.asm -r 4000 -a 8000:5f

test-PHA:
	@echo "Test PHA"
	$(EMU) -c PHA.asm -r 4000 -a 1000:ff

test-PHP:
	@echo "Test PHP"
	$(EMU) -c PHP.asm -r 4000 -a 8000:01

test-PLA:
	@echo "Test PLA"
	$(EMU) -c PLA.asm -r 4000 -a 8000:7f

test-PLP:
	@echo "Test PLP"
	$(EMU) -c PLP.asm -r 4000 -a 8000:01

test-ROL:
	@echo "Test ROL"
	$(EMU) -c ROL.asm -r 4000 -a 8000:aa

test-ROLA:
	@echo "Test ROLA"
	$(EMU) -c ROLA.asm -r 4000 -a 8000:aa

test-ROLX:
	@echo "Test ROLX"
	$(EMU) -c ROLX.asm -r 4000 -a 8000:aa

test-ROLZ:
	@echo "Test ROLZ"
	$(EMU) -c ROLZ.asm -r 4000 -a 8000:aa

test-ROLZX:
	@echo "Test ROLZX"
	$(EMU) -c ROLZX.asm -r 4000 -a 8000:aa

test-ROR:
	@echo "Test ROR"
	$(EMU) -c ROR.asm -r 4000 -a 8000:55

test-RORA:
	@echo "Test RORA"
	$(EMU) -c RORA.asm -r 4000 -a 8000:55

test-RORX:
	@echo "Test RORX"
	$(EMU) -c RORX.asm -r 4000 -a 8000:55

test-RORZ:
	@echo "Test RORZ"
	$(EMU) -c RORZ.asm -r 4000 -a 8000:55

test-RORZX:
	@echo "Test RORZX"
	$(EMU) -c RORZX.asm -r 4000 -a 8000:55

test-RTI:
	@echo "Test RTI"
	$(EMU) -c RTI.asm -r 4000 -a 8000:01

test-RTS:
	@echo "Test RTS"
	$(EMU) -c RTS.asm -r 4000 -a 8000:01

test-SBCA:
	@echo "Test SBCA"
	$(EMU) -c SBCA.asm -r 4000 -a 8000:e0

test-SBCI:
	@echo "Test SBCI"
	$(EMU) -c SBCI.asm -r 4000 -a 8000:50

test-SBCIX:
	@echo "Test SBCIX"
	$(EMU) -c SBCIX.asm -r 4000 -a 8000:e0

test-SBCIY:
	@echo "Test SBCIY"
	$(EMU) -c SBCIY.asm -r 4000 -a 8000:e0

test-SBCX:
	@echo "Test SBCX"
	$(EMU) -c SBCX.asm -r 4000 -a 8000:e0

test-SBCY:
	@echo "Test SBCY"
	$(EMU) -c SBCY.asm -r 4000 -a 8000:e0

test-SBCZ:
	@echo "Test SBCZ"
	$(EMU) -c SBCZ.asm -r 4000 -a 8000:e0

test-SBCZX:
	@echo "Test SBCZX"
	$(EMU) -c SBCZX.asm -r 4000 -a 8000:e0

test-SEC:
	@echo "Test SEC"
	$(EMU) -c SEC.asm -r 4000 -a 8000:01

test-SED:
	@echo "Test SED"
	$(EMU) -c SED.asm -r 4000 -a 8000:01

test-SEI:
	@echo "Test SEI"
	$(EMU) -c SEI.asm -r 4000 -a 8000:01

test-STAA:
	@echo "Test STAA"
	$(EMU) -c STAA.asm -r 4000 -a 8000:7f

test-STAIX:
	@echo "Test STAIX"
	$(EMU) -c STAIX.asm -r 4000 -a 8000:7f

test-STAIY:
	@echo "Test STAIY"
	$(EMU) -c STAIY.asm -r 4000 -a 8000:7f

test-STAX:
	@echo "Test STAX"
	$(EMU) -c STAX.asm -r 4000 -a 8000:7f

test-STAY:
	@echo "Test STAY"
	$(EMU) -c STAY.asm -r 4000 -a 8000:7f

test-STAZ:
	@echo "Test STAZ"
	$(EMU) -c STAZ.asm -r 4000 -a 8000:7f

test-STAZX:
	@echo "Test STAZX"
	$(EMU) -c STAZX.asm -r 4000 -a 8000:7f

test-STXA:
	@echo "Test STXA"
	$(EMU) -c STXA.asm -r 4000 -a 8000:7e

test-STXZ:
	@echo "Test STXZ"
	$(EMU) -c STXZ.asm -r 4000 -a 8000:7e

test-STXZY:
	@echo "Test STXZY"
	$(EMU) -c STXZY.asm -r 4000 -a 8000:7e

test-STYA:
	@echo "Test STYA"
	$(EMU) -c STYA.asm -r 4000 -a 8000:5a

test-STYZ:
	@echo "Test STYZ"
	$(EMU) -c STYZ.asm -r 4000 -a 8000:5a

test-STYZX:
	@echo "Test STYZX"
	$(EMU) -c STYZX.asm -r 4000 -a 8000:5a

test-TAX:
	@echo "Test TAX"
	$(EMU) -c TAX.asm -r 4000 -a 8000:42

test-TAY:
	@echo "Test TAY"
	$(EMU) -c TAY.asm -r 4000 -a 8000:33

test-TSX:
	@echo "Test TSX"
	$(EMU) -c TSX.asm -r 4000 -a 8000:ff

test-TXA:
	@echo "Test TXA"
	$(EMU) -c TXA.asm -r 4000 -a 8000:7e

test-TXS:
	@echo "Test TXS"
	$(EMU) -c TXS.asm -r 4000 -a 8000:50

test-TYA:
	@echo "Test TYA"
	$(EMU) -c TYA.asm -r 4000 -a 8000:55

test-test00:
	@echo "Test test00"
	$(EMU) -c test00.asm -r 4000 -a 0200:55 -a 0210:fe

test-test01:
	@echo "Test test01"
	$(EMU) -c test01.asm -r 4000 -a 00a9:aa

test-test05:
	@echo "Test test05"
	$(EMU) -c test05.asm -r 4000 -a 0040:33

test-timing:
	@echo "Running timing integration tests (1..10 Hz)"
	@N_ITER=15; \
	TOTAL_CYCLES=$$((7 * N_ITER + 1)); \
	TOLERANCE=0.10; \
	EMU_CMD_TIMING="$(EMU) -c timing.asm --rate {rate} -r 4000"; \
	case "$$EMU_CMD_TIMING" in \
	  *\{rate\}*) ;; \
	  *) echo "EMU_CMD_TIMING must contain the literal {rate}"; exit 2 ;; \
	esac; \
	for rate in $$(seq 1 10); do \
	  echo "==== Testing rate $${rate} Hz ===="; \
	  cmd=$$(printf "%s" "$$EMU_CMD_TIMING" | sed "s/{rate}/$$rate/g"); \
	  echo "Command: $$cmd"; \
	  start_time=$$(date +%s.%N); \
	  $$cmd; \
	  exit_status=$$?; \
	  end_time=$$(date +%s.%N); \
	  measured=$$(awk -v s="$$start_time" -v e="$$end_time" 'BEGIN{printf "%.6f", e - s}'); \
	  expected=$$(awk -v c="$$TOTAL_CYCLES" -v r="$$rate" 'BEGIN{printf "%.6f", c / r}'); \
	  lower=$$(awk -v x="$$expected" -v t="$$TOLERANCE" 'BEGIN{printf "%.6f", x * (1 - t)}'); \
	  upper=$$(awk -v x="$$expected" -v t="$$TOLERANCE" 'BEGIN{printf "%.6f", x * (1 + t)}'); \
	  echo "Exit status: $$exit_status"; \
	  echo "Measured time: $${measured}s"; \
	  echo "Expected time: $${expected}s (allowed range: $${lower}s - $${upper}s)"; \
	  ok=$$(awk -v m="$$measured" -v lo="$$lower" -v hi="$$upper" 'BEGIN{print (m+0 >= lo && m+0 <= hi) ? "yes" : "no"}'); \
	  if [ "$$ok" = "yes" ]; then \
	    echo "Result: PASS"; \
	  else \
	    echo "Result: FAIL"; \
	    echo "Timing test failed for rate $${rate} Hz"; \
	    exit 1; \
	  fi; \
	  echo; \
	done; \
	echo "All timing integration tests passed"


# Phase 1 Edge Case Tests - BCD Arithmetic
test-BCD-ADC-01:
	@echo "Test BCD-ADC-01"
	$(EMU) -c BCD-ADC-01.asm -r 4000 -a 8000:10

test-BCD-ADC-02:
	@echo "Test BCD-ADC-02"
	$(EMU) -c BCD-ADC-02.asm -r 4000 -a 8000:00

test-BCD-ADC-03:
	@echo "Test BCD-ADC-03"
	$(EMU) -c BCD-ADC-03.asm -r 4000 -a 8000:04

test-BCD-ADC-04:
	@echo "Test BCD-ADC-04"
	$(EMU) -c BCD-ADC-04.asm -r 4000 -a 8000:41

test-BCD-ADC-05:
	@echo "Test BCD-ADC-05"
	$(EMU) -c BCD-ADC-05.asm -r 4000 -a 8000:01

test-BCD-ADC-06:
	@echo "Test BCD-ADC-06"
	$(EMU) -c BCD-ADC-06.asm -r 4000 -a 8000:98

test-BCD-SBC-01:
	@echo "Test BCD-SBC-01"
	$(EMU) -c BCD-SBC-01.asm -r 4000 -a 8000:09

test-BCD-SBC-02:
	@echo "Test BCD-SBC-02"
	$(EMU) -c BCD-SBC-02.asm -r 4000 -a 8000:97

test-BCD-SBC-03:
	@echo "Test BCD-SBC-03"
	$(EMU) -c BCD-SBC-03.asm -r 4000 -a 8000:00

test-BCD-SBC-04:
	@echo "Test BCD-SBC-04"
	$(EMU) -c BCD-SBC-04.asm -r 4000 -a 8000:01

test-BCD-ADC-ZP:
	@echo "Test BCD-ADC-ZP"
	$(EMU) -c BCD-ADC-ZP.asm -r 4000 -a 8000:01

# NMOS 6502 decimal mode flag quirks
test-BCD-FLAG-NV-01:
	@echo "Test BCD-FLAG-NV-01"
	$(EMU) -c BCD-FLAG-NV-01.asm -r 4000 -a 8000:01

test-BCD-FLAG-NV-02:
	@echo "Test BCD-FLAG-NV-02"
	$(EMU) -c BCD-FLAG-NV-02.asm -r 4000 -a 8000:01

test-BCD-FLAG-Z:
	@echo "Test BCD-FLAG-Z"
	$(EMU) -c BCD-FLAG-Z.asm -r 4000 -a 8000:01

test-BCD-SBC-FLAGS:
	@echo "Test BCD-SBC-FLAGS"
	$(EMU) -c BCD-SBC-FLAGS.asm -r 4000 -a 8000:01

# Phase 1 Edge Case Tests - Page Boundary Crossing
test-LDAX-PAGE:
	@echo "Test LDAX-PAGE"
	$(EMU) -c LDAX-PAGE.asm -r 4000 -a 8000:42

test-LDAY-PAGE:
	@echo "Test LDAY-PAGE"
	$(EMU) -c LDAY-PAGE.asm -r 4000 -a 8000:55

test-STAIX-PAGE:
	@echo "Test STAIX-PAGE"
	$(EMU) -c STAIX-PAGE.asm -r 4000 -a 8000:aa

test-LDAIY-PAGE:
	@echo "Test LDAIY-PAGE"
	$(EMU) -c LDAIY-PAGE.asm -r 4000 -a 8000:66

test-STAX-PAGE:
	@echo "Test STAX-PAGE"
	$(EMU) -c STAX-PAGE.asm -r 4000 -a 8000:77

test-STAY-PAGE:
	@echo "Test STAY-PAGE"
	$(EMU) -c STAY-PAGE.asm -r 4000 -a 8000:88

# Phase 1 Edge Case Tests - Zero Page Wraparound
test-LDAZX-WRAP:
	@echo "Test LDAZX-WRAP"
	$(EMU) -c LDAZX-WRAP.asm -r 4000 -a 8000:33

test-LDAZY-WRAP:
	@echo "Test LDAZY-WRAP"
	$(EMU) -c LDAZY-WRAP.asm -r 4000 -a 8000:44

test-STAZX-WRAP:
	@echo "Test STAZX-WRAP"
	$(EMU) -c STAZX-WRAP.asm -r 4000 -a 8000:55

test-STXZY-WRAP:
	@echo "Test STXZY-WRAP"
	$(EMU) -c STXZY-WRAP.asm -r 4000 -a 8000:66

test-STYZX-WRAP:
	@echo "Test STYZX-WRAP"
	$(EMU) -c STYZX-WRAP.asm -r 4000 -a 8000:77

test-ANDIX-WRAP:
	@echo "Test ANDIX-WRAP"
	$(EMU) -c ANDIX-WRAP.asm -r 4000 -a 8000:aa

# Phase 1 Edge Case Tests - JMP Indirect Bug
test-JMPI-BUG-01:
	@echo "Test JMPI-BUG-01"
	$(EMU) -c JMPI-BUG-01.asm -r 4000 -a 8000:01

test-JMPI-BUG-02:
	@echo "Test JMPI-BUG-02"
	$(EMU) -c JMPI-BUG-02.asm -r 4000 -a 8000:01

test-JMPI-BUG-03:
	@echo "Test JMPI-BUG-03"
	$(EMU) -c JMPI-BUG-03.asm -r 4000 -a 8000:01

# Phase 2 Edge Case Tests - Stack Operations
test-STACK-WRAP-01:
	@echo "Test STACK-WRAP-01"
	$(EMU) -c STACK-WRAP-01.asm -r 4000 -a 8000:ff

test-STACK-WRAP-02:
	@echo "Test STACK-WRAP-02"
	$(EMU) -c STACK-WRAP-02.asm -r 4000 -a 8000:00

test-STACK-JSR-01:
	@echo "Test STACK-JSR-01"
	$(EMU) -c STACK-JSR-01.asm -r 4000 -a 8000:42

test-STACK-PRESERVE-01:
	@echo "Test STACK-PRESERVE-01"
	$(EMU) -c STACK-PRESERVE-01.asm -r 4000 -a 8000:01

test-STACK-FULL-01:
	@echo "Test STACK-FULL-01"
	$(EMU) -c STACK-FULL-01.asm -r 4000 -a 8000:01

# Phase 2 Edge Case Tests - Flag Interactions
test-FLAG-ADC-NVZ:
	@echo "Test FLAG-ADC-NVZ"
	$(EMU) -c FLAG-ADC-NVZ.asm -r 4000 -a 8000:01

test-FLAG-SBC-BORROW:
	@echo "Test FLAG-SBC-BORROW"
	$(EMU) -c FLAG-SBC-BORROW.asm -r 4000 -a 8000:01

test-FLAG-OVERFLOW-01:
	@echo "Test FLAG-OVERFLOW-01"
	$(EMU) -c FLAG-OVERFLOW-01.asm -r 4000 -a 8000:01

test-FLAG-OVERFLOW-02:
	@echo "Test FLAG-OVERFLOW-02"
	$(EMU) -c FLAG-OVERFLOW-02.asm -r 4000 -a 8000:01

test-FLAG-ZERO-ADC:
	@echo "Test FLAG-ZERO-ADC"
	$(EMU) -c FLAG-ZERO-ADC.asm -r 4000 -a 8000:01

test-FLAG-ZERO-SBC:
	@echo "Test FLAG-ZERO-SBC"
	$(EMU) -c FLAG-ZERO-SBC.asm -r 4000 -a 8000:01

# Phase 2 Edge Case Tests - Boundary Values
test-BOUNDARY-INC-FF:
	@echo "Test BOUNDARY-INC-FF"
	$(EMU) -c BOUNDARY-INC-FF.asm -r 4000 -a 8000:01

test-BOUNDARY-DEC-00:
	@echo "Test BOUNDARY-DEC-00"
	$(EMU) -c BOUNDARY-DEC-00.asm -r 4000 -a 8000:01

test-BOUNDARY-ADC-7F:
	@echo "Test BOUNDARY-ADC-7F"
	$(EMU) -c BOUNDARY-ADC-7F.asm -r 4000 -a 8000:01

test-BOUNDARY-SBC-80:
	@echo "Test BOUNDARY-SBC-80"
	$(EMU) -c BOUNDARY-SBC-80.asm -r 4000 -a 8000:01

test-BOUNDARY-ASL-80:
	@echo "Test BOUNDARY-ASL-80"
	$(EMU) -c BOUNDARY-ASL-80.asm -r 4000 -a 8000:01

test-BOUNDARY-LSR-01:
	@echo "Test BOUNDARY-LSR-01"
	$(EMU) -c BOUNDARY-LSR-01.asm -r 4000 -a 8000:01

test-BOUNDARY-ROL-FF:
	@echo "Test BOUNDARY-ROL-FF"
	$(EMU) -c BOUNDARY-ROL-FF.asm -r 4000 -a 8000:01

test-OPCODE-ADC-61:
	@echo "Test OPCODE-ADC-61"
	$(EMU) -c OPCODE-ADC-61.asm -r 4000 -a 8000:41

test-OPCODE-ADC-75:
	@echo "Test OPCODE-ADC-75"
	$(EMU) -c OPCODE-ADC-75.asm -r 4000 -a 8000:50

test-OPCODE-AND-21:
	@echo "Test OPCODE-AND-21"
	$(EMU) -c OPCODE-AND-21.asm -r 4000 -a 8000:11

test-OPCODE-AND-35:
	@echo "Test OPCODE-AND-35"
	$(EMU) -c OPCODE-AND-35.asm -r 4000 -a 8000:20

test-OPCODE-SBC-E1:
	@echo "Test OPCODE-SBC-E1"
	$(EMU) -c OPCODE-SBC-E1.asm -r 4000 -a 8000:1f

test-OPCODE-SBC-F5:
	@echo "Test OPCODE-SBC-F5"
	$(EMU) -c OPCODE-SBC-F5.asm -r 4000 -a 8000:10

test-ZPWRAP-LDAIY:
	@echo "Test ZPWRAP-LDAIY"
	$(EMU) -c ZPWRAP-LDAIY.asm -r 4000 -a 8000:5a

test-ZPWRAP-LDAIX:
	@echo "Test ZPWRAP-LDAIX"
	$(EMU) -c ZPWRAP-LDAIX.asm -r 4000 -a 8000:5a

test-ZPWRAP-ADCIY:
	@echo "Test ZPWRAP-ADCIY"
	$(EMU) -c ZPWRAP-ADCIY.asm -r 4000 -a 8000:41

test-ZPWRAP-SBCIX:
	@echo "Test ZPWRAP-SBCIX"
	$(EMU) -c ZPWRAP-SBCIX.asm -r 4000 -a 8000:1f

test-ZPWRAP-STAIY:
	@echo "Test ZPWRAP-STAIY"
	$(EMU) -c ZPWRAP-STAIY.asm -r 4000 -a 8000:5a

test-ZPWRAP-STAIX:
	@echo "Test ZPWRAP-STAIX"
	$(EMU) -c ZPWRAP-STAIX.asm -r 4000 -a 8000:5a

test-ADDRWRAP-LDAX:
	@echo "Test ADDRWRAP-LDAX"
	$(EMU) -c ADDRWRAP-LDAX.asm -r 4000 -a 8000:5a

test-ADDRWRAP-LDAY:
	@echo "Test ADDRWRAP-LDAY"
	$(EMU) -c ADDRWRAP-LDAY.asm -r 4000 -a 8000:5a

test-ADDRWRAP-STAX:
	@echo "Test ADDRWRAP-STAX"
	$(EMU) -c ADDRWRAP-STAX.asm -r 4000 -a 8000:5a

test-ADDRWRAP-INCX:
	@echo "Test ADDRWRAP-INCX"
	$(EMU) -c ADDRWRAP-INCX.asm -r 4000 -a 8000:42

test-ADDRWRAP-LDAIY:
	@echo "Test ADDRWRAP-LDAIY"
	$(EMU) -c ADDRWRAP-LDAIY.asm -r 4000 -a 8000:5a

test-ADDRWRAP-STAIY:
	@echo "Test ADDRWRAP-STAIY"
	$(EMU) -c ADDRWRAP-STAIY.asm -r 4000 -a 8000:5a

test-DATA-PREFIX:
	@echo "Test DATA-PREFIX"
	$(EMU) -c DATA-PREFIX.asm -r 4000 -a 8000:01
