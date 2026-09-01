# Batch-mode GDB script used to capture docs/gdb_session.txt.
# Run via: make debug (in one terminal) then, in another:
#   arm-none-eabi-gdb -q build/firmware.elf -x scripts/gdb_session.gdb
target remote :1234
break protocol_decode
continue
print frame_len
print/x frame[0]
print frame[1]
backtrace
finish
continue
print frame_len
backtrace
finish
detach
quit
