#!/bin/bash
# Negative tests for the assembler.
#
# Every fail/*.asm must FAIL to assemble (nonzero exit from `6502 -c`). If a
# matching fail/<name>.err exists, its contents must also appear as a
# substring of the assembler's combined output.
#
# EMU can be set to the emulator binary; it defaults to the debug build for
# this platform.

cd "$(dirname "$0")" || exit 1

if [ -z "$EMU" ]; then
    case "$(uname)" in
        Darwin) PLATFORM=macos ;;
        *)      PLATFORM=linux ;;
    esac
    EMU=../bin/debug/$PLATFORM/6502
fi

if [ ! -x "$EMU" ]; then
    echo "fail.sh: emulator not found at $EMU (build first)" >&2
    exit 2
fi

PASSED=0
FAILED=0
FAILED_TESTS=()

for src in fail/*.asm; do
    # Skip "NAME 2.asm" and the like: conflict copies a syncing service
    # (iCloud, Dropbox) leaves next to the real file. No test has a space
    # in its name.
    case "$src" in *" "*) continue ;; esac

    name=$(basename "$src" .asm)
    err="fail/$name.err"
    output=$("$EMU" -c "$src" 2>&1)
    status=$?

    if [ $status -eq 0 ]; then
        echo "✗ $name (assembled successfully, expected failure)"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$name")
    elif [ -f "$err" ] && ! grep -qF -- "$(cat "$err")" <<< "$output"; then
        echo "✗ $name (expected output to contain: $(cat "$err"))"
        echo "$output"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$name")
    else
        PASSED=$((PASSED + 1))
    fi
done

echo "Fail: $PASSED passed, $FAILED failed"
if [ $FAILED -gt 0 ]; then
    echo "Failed: ${FAILED_TESTS[*]}"
    exit 1
fi
