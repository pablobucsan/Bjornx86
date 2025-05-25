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

_main:
    push rbp
    mov rbp, rsp
    ;Align to 16 bytes
    sub rsp, 8
    mov al, 10
    mov byte [rbp - 1], al
    mov al, 65
    mov byte [rbp - 2], al
    mov al, 66
    mov byte [rbp - 2], al
    ; Preparing to call function print()
    mov dil, [rbp - 2]
    call _print_c

    ;Initial assignment of for loop
    mov al, 0
    mov byte [rbp - 3], al

;For loop
.for_loop_1:
    ;Condition check
    mov al, [rbp - 3]
    mov bl, 5
    cmp al, bl
    setl cl
    mov al, cl
    cmp al, 0
    je .end_.for_loop_1
    ;Body
    mov al, [rbp - 1]
    mov bl, [rbp - 3]
    sub al , bl
    mov byte [rbp - 1], al
    ; Preparing to call function print()
    mov dil, [rbp - 1]
    call _print_i8
    ;Reassignment for index
    mov al, 1
    mov byte [rbp - 3], al
    ;Back to condition check
    jmp .for_loop_1

.end_.for_loop_1:
    ;Restore the stack pointer
    add rsp, 8
    pop rbp
    ret

