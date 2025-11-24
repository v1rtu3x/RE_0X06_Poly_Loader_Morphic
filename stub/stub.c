// stub/stub.c 

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

extern unsigned char __packed_start[];
extern unsigned char __packed_end[];

struct packer_meta {
    uint32_t payload_size;   
    uint8_t  rot;            
    uint8_t  pad1;           
    uint16_t pad2;           
    uint32_t expected_sum;   
    uint64_t user_key_hash;  
};

__attribute__((section(".packmeta")))
struct packer_meta g_meta = {0, 0, 0, 0, 0, 0};

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

static uint8_t derive_key(uint32_t payload_size, uint32_t expected_sum) {
    uint32_t x = payload_size ^ expected_sum ^ 0xA5A5A5A5u;
    uint8_t k = (uint8_t)(x & 0xFF);
    return (uint8_t)((k << 3) | (k >> 5));
}

static uint64_t fnv1a64(const uint8_t *buf, size_t len) {
    uint64_t h = 0xcbf29ce484222325ULL;      
    const uint64_t prime = 0x100000001b3ULL; 

    for (size_t i = 0; i < len; i++) {
        h ^= buf[i];
        h *= prime;
    }
    return h;
}

static void *alloc_rw(size_t size) {
    long pagesz = sysconf(_SC_PAGESIZE);
    if (pagesz <= 0) {
        pagesz = 4096;
    }

    size_t ps = (size_t)pagesz;
    size_t alloc_size = (size + ps - 1) & ~(ps - 1);

    void *p = mmap(NULL, alloc_size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS,
                   -1, 0);
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
    size_t alloc_size = (size + ps - 1) & ~(ps - 1);

    if (mprotect(p, alloc_size, PROT_READ | PROT_EXEC) != 0) {
        perror("mprotect");
        return -1;
    }
    return 0;
}

static int ask_and_check_key(uint64_t expected_hash) {
    char buf[128];

    printf("Enter key: ");
    fflush(stdout);

    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n <= 0) {
        fprintf(stderr, "No input\n");
        return -1;
    }

    buf[n] = '\0';

    size_t len = 0;
    while (buf[len] && buf[len] != '\n' && buf[len] != '\r') {
        len++;
    }
    buf[len] = '\0';

    uint64_t h = fnv1a64((const uint8_t *)buf, len);
    if (h != expected_hash) {
        fprintf(stderr, "Wrong key\n");
        return -1;
    }

    return 0;
}

int main(void) {
    size_t payload_size = g_meta.payload_size;
    if (payload_size == 0) {
        fprintf(stderr, "Invalid metadata: payload_size=0\n");
        return 1;
    }

    uint8_t *packed_start = __packed_start;
    size_t packed_section_size = (size_t)(__packed_end - __packed_start);
    if (payload_size > packed_section_size) {
        fprintf(stderr,
                "Invalid metadata: payload_size=%zu > packed_section_size=%zu\n",
                payload_size, packed_section_size);
        return 1;
    }

    uint8_t rot = g_meta.rot;
    uint32_t expected_sum = g_meta.expected_sum;
    uint64_t key_hash = g_meta.user_key_hash;

    uint8_t real_key = derive_key((uint32_t)payload_size, expected_sum);

    if (ask_and_check_key(key_hash) != 0) {
        return 1;  
    }

    void *buf = alloc_rw(payload_size);
    if (!buf) {
        return 1;
    }

    uint8_t *out = (uint8_t *)buf;
    for (size_t i = 0; i < payload_size; i++) {
        uint8_t c = packed_start[i];
        c = ror(c, rot);
        c ^= real_key;
        out[i] = c;
    }

    uint32_t sum = checksum(out, payload_size);
    if (sum != expected_sum) {
        fprintf(stderr,
                "Integrity check failed (got 0x%08x, expected 0x%08x)\n",
                sum, expected_sum);
        volatile int *crash = (int *)0x1;
        *crash = 1;  
        return 1;
    }

    if (make_rx(buf, payload_size) != 0) {
        return 1;
    }

    void (*entry)(void) = (void (*)(void))buf;
    entry();  

    return 0;
}
