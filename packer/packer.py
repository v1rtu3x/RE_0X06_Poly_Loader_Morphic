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

SECRET_KEY = b"rwx_roulette_master_key"

def read_section_info(binary: Path, section_name: str):
    out = subprocess.check_output(
        ["readelf", "-S", "-W", str(binary)],
        text=True,
    )
    for line in out.splitlines():
        parts = line.split()
        if len(parts) < 7:
            continue
        if parts[1] == section_name:
            off = int(parts[4], 16)
            size = int(parts[5], 16)
            return off, size
    raise RuntimeError(f"Section {section_name} not found in {binary}")

def rol8(v, r):
    r &= 7
    v &= 0xFF
    return ((v << r) | (v >> (8 - r))) & 0xFF

def fnv1a64(data: bytes) -> int:
    h = 0xcbf29ce484222325
    prime = 0x100000001b3
    for b in data:
        h ^= b
        h = (h * prime) & 0xFFFFFFFFFFFFFFFF
    return h

def derive_key(payload_size: int, expected_sum: int) -> int:
    """Must match derive_key() in stub.c."""
    x = (payload_size ^ expected_sum ^ 0xA5A5A5A5) & 0xFFFFFFFF
    k = x & 0xFF
    return ((k << 3) | (k >> 5)) & 0xFF

def encode_payload(data, key, rot):
    out = bytearray(len(data))
    for i, b in enumerate(data):
        out[i] = rol8(b ^ key, rot)
    return bytes(out)

def main():
    if not PAYLOAD_BIN.exists():
        raise SystemExit(f"Payload not found: {PAYLOAD_BIN}")
    if not STUB_TEMPLATE.exists():
        raise SystemExit(f"Stub template not found: {STUB_TEMPLATE}")

    payload = PAYLOAD_BIN.read_bytes()
    payload_size = len(payload)
    expected_sum = sum(payload) & 0xFFFFFFFF

    rot = random.randrange(1, 8)
    real_key = derive_key(payload_size, expected_sum)

    user_key_hash = fnv1a64(SECRET_KEY)

    print(f"[*] Payload size: {payload_size} bytes")
    print(f"[*] Derived real XOR key (runtime): 0x{real_key:02x}")
    print(f"[*] rot: {rot}")
    print(f"[*] Expected checksum: 0x{expected_sum:08x}")
    print(f"[*] User key string: {SECRET_KEY!r}")
    print(f"[*] User key hash (stored, 64-bit): 0x{user_key_hash:016x}")

    encoded_payload = encode_payload(payload, real_key, rot)

    stub_bytes = bytearray(STUB_TEMPLATE.read_bytes())

    packed_off, packed_size = read_section_info(STUB_TEMPLATE, ".packed")
    meta_off, meta_size = read_section_info(STUB_TEMPLATE, ".packmeta")

    if payload_size > packed_size:
        raise SystemExit(
            f"Payload ({payload_size} bytes) does not fit in .packed ({packed_size} bytes)"
        )

    stub_bytes[packed_off : packed_off + payload_size] = encoded_payload

    meta_struct = struct.pack(
        "<IBBHI4xQ",
        payload_size,    
        rot,             
        0,              
        0,               
        expected_sum,    
        user_key_hash,   
    )

    if meta_size < len(meta_struct):
        raise SystemExit(
            f".packmeta too small ({meta_size} bytes), need {len(meta_struct)} bytes"
        )

    stub_bytes[meta_off : meta_off + len(meta_struct)] = meta_struct

    CHALLENGE_DIR.mkdir(exist_ok=True)
    CHALLENGE_BIN.write_bytes(stub_bytes)
    os.chmod(CHALLENGE_BIN, 0o755)

    print(f"[*] Packed successfully into {CHALLENGE_BIN}")

if __name__ == "__main__":
    main()
