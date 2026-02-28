section .data
STDOUT_struct:
    dq 0
    dq 0
    dd 1
    dd 1
    dd 0
STDOUT:
    dq STDOUT_struct



section .text

; Extern functions 

global _start
global _main_u64_ppc
_start:
    mov rbp, qword rsp
    mov rdi, qword [rbp]
    lea rsi, qword [rbp + 8]
    call _main_u64_ppc

    ;Exit execution

    mov rax, 60
    mov rdi, 0
    syscall

_Nolose:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov rax, qword [rbp - 8]
    mov eax, dword [rax]
    mov bl, 64
    movzx ebx, bl
    add ebx, eax
    mov eax, ebx
    jmp ret_from_Nolose
ret_from_Nolose:
    add rsp, 8
    pop rbp
    ret
_main_u64_ppc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
ret_from_main_u64_ppc:
    add rsp, 24
    pop rbp
    ret
