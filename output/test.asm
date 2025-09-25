section .data



section .text

; Extern functions 
extern _print_u8
extern _print_u16
extern _print_u32
extern _print_i8
extern _print_i16
extern _print_i32
extern _print_c
extern _print_s

global _start
_start:
    call _main

    ;Exit execution

    mov rax, 60
    mov rdi, 0
    syscall

_fib_u8:
    push rbp
    mov rbp, rsp
    ;Align to 16 bytes
    sub rsp, 24
    mov [rbp - 24], dil 

;If statement
.if_stmt_1:
    mov al, [rbp - 24]
    mov bl, 1
    cmp al, bl
    setle cl
    mov al, cl
    cmp al, 0
    je .end_.if_stmt_1
    ; Return node
    mov eax, 1
    jmp .ret_from_fib_u8
    jmp .end_.if_stmt_1
.end_.if_stmt_1:
    mov eax, 0
    mov dword [rbp - 20], eax
    mov eax, 1
    mov dword [rbp - 16], eax
    mov al, 2
    mov byte [rbp - 12], al
    mov dword [rbp - 8], 0

;While loop
.while_loop_1:
    ;Condition check
    mov al, [rbp - 12]
    mov bl, [rbp - 24]
    cmp al, bl
    setle cl
    mov al, cl
    cmp al, 0
    je .end_.while_loop_1
    ;Body
    lea rax, [rbp - 8]
    mov ebx, [rbp - 20]
    mov ecx, [rbp - 16]
    add ebx, ecx
    mov dword [rax], ebx 
    lea rax, [rbp - 20]
    mov ebx, [rbp - 16]
    mov dword [rax], ebx 
    lea rax, [rbp - 16]
    mov ebx, [rbp - 8]
    mov dword [rax], ebx 
    lea rax, [rbp - 12]
    mov bl, [rbp - 12]
    mov cl, 1
    add bl, cl
    mov byte [rax], bl 
    ;Back to condition check
    jmp .while_loop_1

.end_.while_loop_1:
    ; Return node
    mov eax, [rbp - 8]
    jmp .ret_from_fib_u8
    ;Restore the stack pointer and return
.ret_from_fib_u8:
    add rsp, 24
    pop rbp
    ret

_main:
    push rbp
    mov rbp, rsp
    ;Align to 16 bytes
    sub rsp, 8
    mov al, 40
    mov byte [rbp - 8], al
    ; Preparing to call function fib()
    mov dil, [rbp - 8]
    call _fib_u8
    mov dword [rbp - 4], eax
    ; Preparing to call function print()
    mov edi, [rbp - 4]
    call _print_u32
    ;Restore the stack pointer and return
.ret_from_main:
    add rsp, 8
    pop rbp
    ret

