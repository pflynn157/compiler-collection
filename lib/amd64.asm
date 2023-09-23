# For now, we just call the main function and
# pass parameters
.intel_syntax noprefix

.text
.global _start
.global invoke_syscall
.global malloc
.global free
.global debug
.global realloc
.extern main

_start:
    xor ebp, ebp
    mov esi, DWORD PTR [rsp+0]
    lea rdi, 8[rsp]
    call main
    
    mov rdi, rax
    mov rax, 60
    syscall

invoke_syscall:
    mov rax, rdi
    mov rdi, rsi
    mov rsi, rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    syscall
    ret

malloc:
    mov rsi, rdi
    mov rax, 9
    mov rdi, 0
    mov rdx, 3
    mov r10, 34
    mov r8, -1
    mov r9, 0
    syscall
    ret

/* unsigned long addr       rdi
   unsigned long old_len    rsi
   unsigned long new_len    rdx
   unsigned long flags      = 0
   unsigned long new_addr   = 9
*/ 
realloc:
    xor r10, r10
    xor r8, r8
    mov rax, 25
    syscall
    ret

free:
    mov rax, 11
    syscall
    ret

debug:
    mov rdx, rsi
    mov rsi, rdi
    mov rax, 1
    mov rdi, 1
    syscall
    ret

