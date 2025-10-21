
include ../include.mk

# Path to the emulator binary (in parent directory's bin directory)
EMU = ../$(BINDIR)/6502

# Individual test targets based on unittest.script
.PHONY: test test-ADCI test-LDAI1 test-LDAI2 test-LDAI3 test-CLC test-SEC test-NOP test-PHA test-test00 test-test01 test-test05 test-timing

# Run all tests (using - prefix to continue on failure)
test:
	@$(MAKE) test-ADCI || true
	@$(MAKE) test-LDAI1 || true
	@$(MAKE) test-LDAI2 || true
	@$(MAKE) test-LDAI3 || true
	@$(MAKE) test-CLC || true
	@$(MAKE) test-SEC || true
	@$(MAKE) test-NOP || true
	@$(MAKE) test-PHA || true
	@$(MAKE) test-test00 || true
	@$(MAKE) test-test01 || true
	@$(MAKE) test-test05 || true
	@$(MAKE) test-timing || true

test-ADCI:
	@echo "Test ADCI"
	$(EMU) -c ADCI.asm -r 4000 -a 8000:ff

test-LDAI1:
	@echo "Test LDA1"
	$(EMU) -c LDAI1.asm -r 4000 -a 8000:7f

test-LDAI2:
	@echo "Test LDA2"
	$(EMU) -c LDAI2.asm -r 4000 -a 8000:7f

test-LDAI3:
	@echo "Test LDA3"
	$(EMU) -c LDAI3.asm -r 4000 -a 8000:00

test-CLC:
	@echo "Test CLC"
	$(EMU) -c CLC.asm -r 4000 -a 8000:01

test-SEC:
	@echo "Test SEC"
	$(EMU) -c SEC.asm -r 4000 -a 8000:01

test-NOP:
	@echo "Test NOP"
	$(EMU) -c NOP.asm -r 4000 -a 8000:01

test-PHA:
	@echo "Test PHA"
	$(EMU) -c PHA.asm -r 4000 -a 1000:ff

test-test00:
	@echo "Test test00"
	$(EMU) -c test00.asm -r 4000 -a 022a:55

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
	EMU_CMD_TIMING="$(EMU) -c timing.asm --rate {rate}"; \
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
