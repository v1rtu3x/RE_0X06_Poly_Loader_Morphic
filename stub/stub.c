#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

extern unsigned char __packed_start[];
extern unsigned char __packed_end[];

uint8_t g_key = 0;
uint8_t g_rot = 0;
uint32_t g_expected_sum = 0;

__attribute__((section(".packed")))
unsigned char g_packed_placeholder[4096] = {0};  

static inline uint8_t ror(uint8_t v, uint8_t r) {
    r &= 7;
    return (uint8_t)((v >> r) | (v << (8 - r)));
}

static uint32_t checksum(const uint8_t *buf, size_t len) {
    uint32_t s = 0;
    for (size_t i = 0; i < len; i++) {
        s += buf[i];
    }
    return s;
}

static void *alloc_rw(size_t size) {
    long pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz <= 0) {
        pagesz = 4096;
    }

    size_t ps = (size_t)pagesz;
    size_t alloc_size = (size + ps - 1) & ~(ps - 1);

    void *p = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    return p;
}

static int make_rx(void *p, size_t size) {
    long pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz <= 0) {
        pagesz = 4096;
    }

    size_t ps = (size_t)pagesz;

    uintptr_t start = (uintptr_t)p;
    uintptr_t base = start & ~(ps - 1);
    uintptr_t end = start + size;
    size_t len = (size_t)((end + ps - 1) & ~(ps - 1)) - base;

    if (mprotect((void *)base, len, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        return -1;
    }
    return 0;
}

int main(void) {
    uint8_t *packed = __packed_start;
    size_t packed_size = (size_t)(__packed_end - __packed_start);

    if (packed_size == 0) {
        fprintf(stderr, "No packed data\n");
        return 1;
    }

    void *buf = alloc_rw(packed_size);
    if (!buf) {
        return 1;
    }

    uint8_t *out = (uint8_t *)buf;
    for (size_t i = 0; i < packed_size; i++) {
        uint8_t c = packed[i];
        c = ror(c, g_rot);
        c ^= g_key;
        out[i] = c;
    }

    uint32_t sum = checksum(out, packed_size);
    if (sum != g_expected_sum) {
        printf("Integrity check failed (got 0x%08x, expected 0x%08x)\n", sum, g_expected_sum);
        volatile int *crash = (int *)0x1;
        *crash = 42;  
        return 1;
    }

    if (make_rx(buf, packed_size) != 0) {
        return 1;
    }

    void (*entry)(void) = (void (*)(void))buf;
    entry();

    return 0;
}
