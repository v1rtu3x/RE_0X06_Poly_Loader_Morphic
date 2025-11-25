#!/usr/bin/env bash

set -euo pipefail

ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
BIN="$ROOT/challenge/Poly_Loader_Morphic"

GOOD_KEY="rwx_roulette_master_key"
BAD_KEY="this_is_definitely_wrong"

echo "======================================"
echo "        Poly Loader - Test Suite      "
echo "======================================"
echo

if [[ ! -f "$BIN" ]]; then
    echo "Challenge binary not found at:"
    echo "   $BIN"
    echo "   Run ./build.sh first."
    exit 1
fi

echo "[*] Challenge binary found at $BIN"
echo

echo "[*] Testing with CORRECT key: '$GOOD_KEY'"

OUTPUT_GOOD=$(printf "%s\n" "$GOOD_KEY" | "$BIN" 2>&1 || true)

echo "----- Output (correct key) -----"
echo "$OUTPUT_GOOD"
echo "--------------------------------"
echo

if echo "$OUTPUT_GOOD" | grep -q "FLAG{"; then
    echo "Correct-key test: PASS (flag printed)"
else
    echo "Correct-key test: FAIL (flag not printed)"
    exit 1
fi

echo "[*] Testing with WRONG key: '$BAD_KEY'"

OUTPUT_BAD=$(printf "%s\n" "$BAD_KEY" | "$BIN" 2>&1 || true)

echo "----- Output (wrong key) -----"
echo "$OUTPUT_BAD"
echo "------------------------------"
echo

if echo "$OUTPUT_BAD" | grep -q "FLAG{"; then
    echo "Wrong-key test: FAIL (flag printed for wrong key!)"
    exit 1
fi

echo "Wrong-key test: PASS (no flag for wrong key)"
echo
echo "All tests passed!"
