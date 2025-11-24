#!/usr/bin/env bash

set -euo pipefail

ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"

echo "======================================"
echo "          POLY LOADER MORPHIC         "
echo "======================================"

echo "[*] Cleaning old build outputs ..."
rm -f "$ROOT/challenge/Poly_Loader_Morphic" || true

echo
echo "=====Building Payload======"
cd "$ROOT/payload"
./build.sh
cd "$ROOT"

echo 
echo "====Building Stub Loader===="
cd "$ROOT/stub"
./build.sh
cd "$ROOT"

echo 
echo "=Packing Payload into Loader="
python3 "$ROOT/packer/packer.py"

if [[ ! -f "$ROOT/challenge/Poly_Loader_Morphic" ]]; then 
    echo "[!] ERROR: Packer did not produce challenge binary."
    exit 1
fi


echo
echo "======================================"
echo "    Build completed successfully!     "
echo "        Final Challene binary:        "
ls -lh "$ROOT/challenge/Poly_Loader_Morphic"
echo "======================================"
