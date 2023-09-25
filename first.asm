.intel_syntax noprefix
.data
    STR1: .string "Hello!\n"
    
.text

.global main
main:
    mov rdi, 5
    mov eax, 60
    syscall
done:
    ret
