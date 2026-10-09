#!/bin/bash
# Golden-file check for the assembler.
#
# Every test/*.asm and ../sample*.asm is assembled and the resulting 64K image
# is dumped with `xxd -a` (autoskip collapses runs of zeros). The dump is
# compared against golden/<name>.hex. Any difference means the assembler's
# output changed.
#
#   bash golden.sh check    # compare against golden files (default)
#   bash golden.sh update   # rewrite golden files from current output
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
    echo "golden.sh: emulator not found at $EMU (build first)" >&2
    exit 2
fi

if ! command -v xxd > /dev/null; then
    echo "golden.sh: xxd is required" >&2
    exit 2
fi

MODE=${1:-check}
GOLDEN=golden
TMP=$(mktemp -d) || exit 2
trap 'rm -rf "$TMP"' EXIT

mkdir -p "$GOLDEN"

PASSED=0
FAILED=0
UPDATED=0
FAILED_TESTS=()

for src in *.asm ../sample*.asm; do
    # Skip "NAME 2.asm" and the like: conflict copies a syncing service
    # (iCloud, Dropbox) leaves next to the real file. No test has a space
    # in its name.
    case "$src" in *" "*) continue ;; esac

    name=$(basename "$src" .asm)
    bin="$TMP/$name.bin"
    actual="$TMP/$name.hex"
    expected="$GOLDEN/$name.hex"

    if ! "$EMU" -c "$src" -s "$bin" > "$TMP/$name.out" 2>&1; then
        echo "✗ $name (failed to assemble)"
        cat "$TMP/$name.out"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$name")
        continue
    fi

    xxd -a "$bin" > "$actual"

    if [ "$MODE" = update ]; then
        cp "$actual" "$expected"
        UPDATED=$((UPDATED + 1))
    elif [ ! -f "$expected" ]; then
        echo "✗ $name (no golden file; run 'make golden-update')"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$name")
    elif diff -u "$expected" "$actual" > "$TMP/$name.diff"; then
        PASSED=$((PASSED + 1))
    else
        echo "✗ $name"
        cat "$TMP/$name.diff"
        FAILED=$((FAILED + 1))
        FAILED_TESTS+=("$name")
    fi
done

if [ "$MODE" = update ]; then
    echo "Updated $UPDATED golden files in test/$GOLDEN"
    exit $(( FAILED > 0 ))
fi

echo "Golden: $PASSED passed, $FAILED failed"
if [ $FAILED -gt 0 ]; then
    echo "Failed: ${FAILED_TESTS[*]}"
    exit 1
fi
