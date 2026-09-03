#!/usr/bin/env bash
#
# Boots the firmware under QEMU headless, captures its UART output for a
# fixed window, and checks it against the behaviour this project's tasks
# actually implement: two producers' frames accepted, and producer B's
# deliberately corrupted 5th frame rejected. This gives a repeatable
# pass/fail check without a human watching the serial console.
set -u

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ELF="$ROOT_DIR/build/firmware.elf"
# Producer B corrupts every 5th frame (seq=4, 700ms period), so the first
# corrupted frame lands around 3.5s in; 6s leaves real margin instead of
# racing the capture window against it.
CAPTURE_SECONDS="${CAPTURE_SECONDS:-6}"
LOG_FILE="$(mktemp -t qemu_test_XXXXXX.log)"

if [ ! -f "$ELF" ]; then
    echo "FAIL: $ELF not found - run 'make' first" >&2
    exit 1
fi

qemu-system-arm -M mps2-an385 -kernel "$ELF" -nographic -serial mon:stdio \
    > "$LOG_FILE" 2>&1 &
QEMU_PID=$!

sleep "$CAPTURE_SECONDS"
kill -9 "$QEMU_PID" 2>/dev/null
wait "$QEMU_PID" 2>/dev/null

FAIL=0

if ! grep -q "Hello from FreeRTOS on QEMU" "$LOG_FILE"; then
    echo "FAIL: no boot banner seen" >&2
    FAIL=1
fi

OK_COUNT=$(grep -c "frame OK from A" "$LOG_FILE")
if [ "$OK_COUNT" -lt 3 ]; then
    echo "FAIL: expected at least 3 accepted frames from producer A, saw $OK_COUNT" >&2
    FAIL=1
fi

if ! grep -q "frame OK from B" "$LOG_FILE"; then
    echo "FAIL: no accepted frame from producer B" >&2
    FAIL=1
fi

if ! grep -q "frame REJECTED (bad checksum)" "$LOG_FILE"; then
    echo "FAIL: the deliberately corrupted frame was not rejected" >&2
    FAIL=1
fi

if [ "$FAIL" -ne 0 ]; then
    echo "--- captured output ---" >&2
    cat "$LOG_FILE" >&2
    rm -f "$LOG_FILE"
    exit 1
fi

echo "PASS: boot banner seen, $OK_COUNT+ A frames and B frames accepted, corrupted frame rejected"
rm -f "$LOG_FILE"
exit 0
