#!/bin/bash
# Test runner with summary reporting

PASSED=0
FAILED=0
FAILED_TESTS=()

run_test() {
    local test_name=$1
    if make -s "$test_name" 2>&1 > /dev/null; then
        PASSED=$((PASSED + 1))
        echo "✓ $test_name"
    else
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$test_name")
        echo "✗ $test_name"
    fi
}

# Run all tests
run_test test-ADCA
run_test test-ADCI
run_test test-ADCIX
run_test test-ADCIY
run_test test-ADCX
run_test test-ADCY
run_test test-ADCZ
run_test test-ADCZX
run_test test-ANDA
run_test test-ANDI
run_test test-ANDIX
run_test test-ANDIY
run_test test-ANDX
run_test test-ANDY
run_test test-ANDZ
run_test test-ANDZX
run_test test-ASL
run_test test-ASLA
run_test test-ASLX
run_test test-ASLZ
run_test test-ASLZX
run_test test-BCC
run_test test-BCS
run_test test-BEQ
run_test test-BIT
run_test test-BITZ
run_test test-BMI
run_test test-BNE
run_test test-BPL
run_test test-BRK
run_test test-BVC
run_test test-BVS
run_test test-CLC
run_test test-CLD
run_test test-CLI
run_test test-CLV
run_test test-CMPA
run_test test-CMPI
run_test test-CMPIX
run_test test-CMPIY
run_test test-CMPX
run_test test-CMPY
run_test test-CMPZ
run_test test-CMPZX
run_test test-CPXA
run_test test-CPXI
run_test test-CPXZ
run_test test-CPYA
run_test test-CPYI
run_test test-CPYZ
run_test test-DECA
run_test test-DECX
run_test test-DECZ
run_test test-DECZX
run_test test-DEX
run_test test-DEY
run_test test-EORA
run_test test-EORI
run_test test-EORIX
run_test test-EORIY
run_test test-EORX
run_test test-EORY
run_test test-EORZ
run_test test-EORZX
run_test test-INCA
run_test test-INCX
run_test test-INCZ
run_test test-INCZX
run_test test-INX
run_test test-INY
run_test test-JMP
run_test test-JMPI
run_test test-JSR
run_test test-LDAA
run_test test-LDAI1
run_test test-LDAI2
run_test test-LDAI3
run_test test-LDAIX
run_test test-LDAIY
run_test test-LDAX
run_test test-LDAY
run_test test-LDAZ
run_test test-LDAZX
run_test test-LDXA
run_test test-LDXY
run_test test-LDXZ
run_test test-LDXZY
run_test test-LDYA
run_test test-LDYX
run_test test-LDYZ
run_test test-LDYZX
run_test test-LSR
run_test test-LSRA
run_test test-LSRX
run_test test-LSRZ
run_test test-LSRZX
run_test test-NOP
run_test test-ORAA
run_test test-ORAI
run_test test-ORAIX
run_test test-ORAIY
run_test test-ORAX
run_test test-ORAY
run_test test-ORAZ
run_test test-ORAZX
run_test test-PHA
run_test test-PHP
run_test test-PLA
run_test test-PLP
run_test test-ROL
run_test test-ROLA
run_test test-ROLX
run_test test-ROLZ
run_test test-ROLZX
run_test test-ROR
run_test test-RORA
run_test test-RORX
run_test test-RORZ
run_test test-RORZX
run_test test-RTI
run_test test-RTS
run_test test-SBCA
run_test test-SBCI
run_test test-SBCIX
run_test test-SBCIY
run_test test-SBCX
run_test test-SBCY
run_test test-SBCZ
run_test test-SBCZX
run_test test-SEC
run_test test-SED
run_test test-SEI
run_test test-STAA
run_test test-STAIX
run_test test-STAIY
run_test test-STAX
run_test test-STAY
run_test test-STAZ
run_test test-STAZX
run_test test-STXA
run_test test-STXZ
run_test test-STXZY
run_test test-STYA
run_test test-STYZ
run_test test-STYZX
run_test test-TAX
run_test test-TAY
run_test test-TSX
run_test test-TXA
run_test test-TXS
run_test test-TYA
run_test test-test00
run_test test-test01
run_test test-test05
run_test test-timing

# Print summary
echo ""
echo "========================================="
echo "Test Summary"
echo "========================================="
echo "Total tests:  $((PASSED + FAILED))"
echo "Passed:       $PASSED"
echo "Failed:       $FAILED"

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo "========================================="
    exit 1
else
    echo "========================================="
    echo "All tests passed! ✓"
    echo "========================================="
    exit 0
fi
