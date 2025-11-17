.intel_syntax noprefix
.section .text
.global payload_entry

payload_entry:
    lea rsi, [rip + msg]
    mov rdi, 1
    mov rdx, len
    mov rax, 1
    syscall

    mov rax, 60
    xor rdi, rdi
    syscall

.section .rodata
msg:
    .ascii "Correct, here is your flag:\n"
    .ascii "FLAG{rwx_roulette_polymorphic_loader}\n"
len = . - msg
