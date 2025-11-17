#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

echo "[*] Assembling payload.asm -> payload.o"
as -o payload.o payload.asm

echo "[*] Extracting .text -> payload.bin"
objcopy --output-target=binary --only-section=.text payload.o payload.bin

echo "[*] Build complete:"
ls -l payload.o payload.bin

echo
echo "[*] First bytes of payload.bin:"
hexdump -C payload.bin | head || true
