.intel_syntax noprefix
.section .text
.global payload_entry

payload_entry:
    lea rsi, [rip + msg]         
    mov edi, 1                   
    mov edx, msg_end - msg      
    mov eax, 1                   
    syscall

    mov eax, 60                 
    xor edi, edi                
    syscall

msg:
    .ascii "Correct, here is your flag:\n"
    .ascii "FLAG{rwx_roulette_polymorphic_loader}\n"
msg_end:
