.intel_syntax noprefix
.data
    STR1: .string "Hello!\n"
    
.text

.global _start
_start:
    mov rdi, 5
    mov eax, 60
    syscall
done:
    ret

.global func1
func1:
    mov rdi, 1
    mov eax, 60
    syscall

.global func2
func2:
    mov rdi, 2
    mov eax, 60
    syscall
    
