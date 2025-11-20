#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

echo "[*] Building stub -> packed_template"
gcc -Wall -Wextra -O2 stub.c -o packed_template -T linker.ld

echo "[*] Result:"
ls -l packed_template
