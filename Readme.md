# Poly Loader Morphic

A self-contained reverse-engineering challenge implementing a polymorphic loader with runtime key derivation, integrity verification, payload unpacking, and execution of position-independent shellcode.

The purpose of this project is to demonstrate and practice:
- Entry-stub analysis  
- Payload unpacking  
- XOR+rotate polymorphic encodings  
- Metadata tampering resistance  
- Loader reversing  
- Key-based gating (FNV-1a 64-bit)  
- Memory permissions and execution flow  
- Working with custom ELF sections  

---

## Features

### 1. Payload (PIShellcode)
A small position-independent assembly routine that:
- Prints a flag
- Exits cleanly  
The payload is compiled to raw `.text` bytes and then encoded by the packer.

### 2. Loader Stub
A C program compiled with a custom linker script. At runtime it:
1. Prompts the user for a key string  
2. Computes its 64-bit FNV-1a hash  
3. Verifies the hash against metadata  
4. Derives the real XOR key from:
   - Payload size  
   - Checksum  
   - A fixed obfuscation constant  
5. Decodes the packed payload  
6. Verifies its checksum  
7. Marks it executable and jumps to entry

Incorrect key → no decode.  
Integrity failure → intentional crash (anti-tamper).

### 3. Packer
A Python script that:
- Reads and encodes the payload  
- Derives the XOR decode key  
- Computes all metadata  
- Writes encoded payload and metadata into the stub template sections  
- Produces the final challenge binary

### 4. Build System
A root-level `build.sh`:
- Cleans old builds  
- Rebuilds payload  
- Rebuilds stub  
- Packs everything  
- Places final binary in `challenge/`

### 5. Test Suite
A non-interactive `test.sh` that:
- Runs the challenge with the correct key  
- Ensures flag prints  
- Runs with a wrong key  
- Ensures no flag is printed  

---

## Repository Layout

```text
RE_0X06_Poly_Loader_Morphic/
│
├── build.sh                     # Root build script
│
├── payload/
│   ├── payload.asm              # Position-independent flag-print assembly
│   └── build.sh                 # Assembles payload.o and extracts payload.bin
│
├── stub/
│   ├── stub.c                   # Loader/unpacker with key check
│   ├── linker.ld                # Custom ELF layout with .packmeta and .packed
│   └── build.sh                 # Builds packed_template
│
├── packer/
│   └── packer.py                # Writes metadata + encoded payload to stub
│
├── challenge/
│   └── Poly_Loader_Morphic      # Final generated challenge binary
│
└── test/
    └── test.sh                  # Automated correct/wrong key test suite

```

## Build Instructions

From the repository root:
```
./build.sh
```
The final binary will be generated at:

```
/challenge/Poly_Loader_Morphic
```

right key:
```
./challenge/Poly_Loader_Morphic
Enter key: rwx_roulette_master_key
Correct, here is your flag:
FLAG{rwx_roulette_polymorphic_loader}
```

Wrong key:


```
Enter key: wrong
Wrong key
```

---

## Testing the Challenge

```
./test/test.sh
```

The script performs:
- Correct-key test → must print flag
- Wrong-key test → must not print flag  

---

## Summary

This project implements a realistic polymorphic loader architecture suitable for reverse-engineering practice and writeups.  
It demonstrates packed payloads, rotating XOR obfuscation, integrity checking, runtime decode, and key-based access control inside a minimal custom ELF loader.
