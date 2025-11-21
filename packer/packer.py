#!/usr/bin/env python3
import os
import struct
import random
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PAYLOAD_BIN = ROOT / "payload" / "payload.bin"
STUB_TEMPLATE = ROOT / "stub" / "packed_template"
CHALLENGE_DIR = ROOT / "challenge"
CHALLENGE_BIN = CHALLENGE_DIR / "Poly_Loader_Morphic"

def read_section_info(binary: Path, section_name: str):
    
    out = subprocess.check_output(
        ["readelf", "-S", "-W", str(binary)], text=True
        )
    
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 7:
            continue
        if parts[1] == section_name:
            file_off_hex = parts[4]
            size_hex = parts[5]
            off = int(file_off_hex, 16)
            size = int(size_hex, 16)
            return off, size
    raise RuntimeError(f"Section {section_name} not found in {binary}")

def rol8(val: int, r: int)-> int: 
    r &= 7
    val = val & 0xFF
    return ((val << r) | (val >> (8 - r))) & 0xFF

def encode_payload(data: bytes, key: int, rot: int) -> bytes:
    out = bytearray(len(data))
    for i, b in enumerate(data):
        x = b ^ key
        out[i] = rol8(x, rot)
    return bytes(out)


def checksum(data: bytes) -> int:
    return sum(data) & 0xFFFFFFFF

def main():
    if not PAYLOAD_BIN.exists():
        raise SystemExit(f"Payload not found: {PAYLOAD_BIN}. Build it first.")
    if not STUB_TEMPLATE.exists():
        raise SystemExit(f"Stub template not found: {STUB_TEMPLATE}. Build it first.")

    print(f"[*] Reading payload from {PAYLOAD_BIN}")
    payload = PAYLOAD_BIN.read_bytes()
    print(f"[*] Payload size: {len(payload)} bytes")

    rnd = random.SystemRandom()
    key = rnd.randrange(1, 256)
    rot = rnd.randrange(1, 8)
    print(f"[*] Using key=0x{key:02x}, rot={rot}")

    encoded = encode_payload(payload, key, rot)
    expected_sum = checksum(payload)
    print(f"[*] Checksum (plain payload): 0x{expected_sum:08x}")

    print(f"[*] Locating sections in {STUB_TEMPLATE}")
    packed_off, packed_size = read_section_info(STUB_TEMPLATE,".packed")
    meta_off, meta_size = read_section_info(STUB_TEMPLATE, ".packmeta")
    print(f" .packed at offset 0x{packed_off:x}, size 0x{packed_size:x}")
    print(f" .packmeta at offset 0x{meta_off:x}, size 0x{meta_size:x}")

    if len(encoded) > packed_size: 
        raise SystemExit(f"Encoded payload ({len(encoded)} bytes) does not fit in .packed ({packed_size} bytes)")
    
    stub_bytes = bytearray(STUB_TEMPLATE.read_bytes())
    stub_bytes[packed_off:packed_off + len(encoded)] = encoded

    meta_struct = struct.pack("<BBHI", key, rot, 0, expected_sum)


    print("[*] Wrote metadata (key/rot/checksum) into .packmeta")

    CHALLENGE_DIR.mkdir(parents=True, exist_ok=True)

    CHALLENGE_BIN.write_bytes(stub_bytes)
    os.chmod(CHALLENGE_BIN, 0o755)

    print(f"[*] Challenge binary written to {CHALLENGE_BIN}")

if __name__ == "__main__":
    main()
