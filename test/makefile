
TESTS=ADCI CLC LDAI1 LDAI2 LDAI3 NOP PHA SEC test00 test01 test02 test03 test04 test05
ASMS = $(TESTS:%=%.asm)

include ../include.mk

test: $(TESTS)

% : %.asm                                                                                                                                                                                
	$(BINDIR)/6502 -c $< -r 4000 -a 8000:01 
