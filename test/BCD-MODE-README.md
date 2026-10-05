# BCD Mode Implementation Note

## Status: NOT IMPLEMENTED ⚠️

The emulator currently **does not implement BCD (Binary-Coded Decimal) mode**. 

### What This Means

- The SED (Set Decimal Mode) and CLD (Clear Decimal Mode) instructions are recognized but have no effect
- ADC and SBC operations in decimal mode perform **binary arithmetic** instead of BCD arithmetic
- All BCD test files (BCD-ADC-*.asm, BCD-SBC-*.asm) will **FAIL** until BCD mode is implemented

### Expected Behavior

In true 6502 BCD mode:
- Each byte is treated as two decimal digits (0-9)
- ADC #$09 + #$01 should give $10 (not $0A)
- ADC #$99 + #$01 should give $00 with carry set
- Invalid BCD digits ($0A-$0F) produce undefined behavior

### Test Files

The following test files require BCD mode implementation:
- BCD-ADC-01.asm - Basic BCD addition ($09 + $01 = $10)
- BCD-ADC-02.asm - BCD carry generation ($99 + $01 = $00)
- BCD-ADC-03.asm - Multi-digit carry ($58 + $46 = $04 + carry)
- BCD-ADC-04.asm - BCD with carry in ($15 + $25 + 1 = $41)
- BCD-ADC-05.asm - Zero edge case ($00 + $00 = $00)
- BCD-ADC-06.asm - Maximum digits ($99 + $99 = $98 + carry)
- BCD-SBC-01.asm - Basic BCD subtraction ($25 - $16 = $09)
- BCD-SBC-02.asm - BCD borrow generation ($32 - $35 = $97 with borrow)
- BCD-SBC-03.asm - Zero result ($50 - $50 = $00)

### Implementation Priority

BCD mode is **LOW PRIORITY** for most applications:
- Modern software rarely uses BCD mode
- Many emulators omit BCD support
- The NMOS 6502 BCD implementation has known bugs that some software depends on

However, for **accurate emulation** of vintage 6502 systems that use BCD (calculators, financial software), this feature should be implemented.

### References

- [6502 Decimal Mode Tutorial](http://www.6502.org/tutorials/decimal_mode.html)
- [BCD Arithmetic Discussion](http://www.6502.org/tutorials/decimal_mode.html#A)
- Known NMOS 6502 BCD bugs and behavior

### To Do

1. Implement decimal mode flag handling in SED/CLD
2. Modify ADC instruction to check decimal mode flag
3. Modify SBC instruction to check decimal mode flag
4. Implement BCD digit adjustment logic
5. Handle invalid BCD values (optional - undefined on real hardware)
6. Decide whether to emulate NMOS bugs or implement "correct" BCD

Once implemented, all BCD tests should pass with `make test`.
