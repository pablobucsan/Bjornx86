section .data
O_RDONLY dd 0
O_WRONLY dd 1
O_RDWR dd 2
O_CREATE dd 64
O_APPEND dd 1024
USER_EXE dd 64
USER_WRITE dd 128
USER_READ dd 256
GROUP_EXE dd 8
GROUP_WRITE dd 16
GROUP_READ dd 32
null dq 0



Lstr0: db 84,104,101,32,115,117,109,32,111,102,32,116,104,101,32,108,105,110,107,101,100,32,108,105,115,116,32,105,115,0
Lstr1: db 84,104,101,32,115,117,109,32,111,102,32,116,104,101,32,97,114,114,97,121,32,105,115,0
Lstr2: db 70,105,98,32,114,101,115,117,108,116,32,105,115,0
section .text

; Extern functions 
extern _putc_c
extern _write_u32_s_u32
extern _print_u8
extern _print_u16
extern _print_u32
extern _print_i8
extern _print_i16
extern _print_i32
extern _open_s_u32_u32
extern _close_u32

global _start
_start:
    call _main

    ;Exit execution

    mov rax, 60
    mov rdi, 0
    syscall

_print_c:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], dil
    ;Done loading parameters
    ;Preparing to call function
    mov al, [rbp - 8]
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Preparing to call function
    mov al, 10
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
.ret_from_print_c:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_s:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 24], rdi
    ;Done loading parameters
    ;Building assignment expression 
    mov eax, 0
    mov [rbp - 16], eax
    ;While loop
.while_loop_1:
    ;While loop condition check
    ;Array subscript
    lea rax, [rbp - 24]
    ;String subscript
    mov rax, [rax]
    ;Computing the index
    mov ebx, [rbp - 16]
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx 
    mov al, [rax]
    ;Array subscript end
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.while_loop_1
    ;Body
    ;Preparing to call function
    ;Array subscript
    lea rax, [rbp - 24]
    ;String subscript
    mov rax, [rax]
    ;Computing the index
    lea rbx, [rbp - 16]
    mov ecx, [rbx]
    inc dword [rbx] 
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 1 
    add rax, rcx 
    mov al, [rax]
    ;Array subscript end
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Back to while loop condition check
    jmp .while_loop_1

.end_.while_loop_1:
    ;Preparing to call function
    mov al, 10
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
.ret_from_print_s:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_printf_s:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
.ret_from_printf_s:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_sum_pNode:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
    ;If statement
.if_stmt_1:
    ;Condition check of if block
    mov rax, [rbp - 8]
    mov rbx, [null] 
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_1
    ;Return node
    mov al, 0
    jmp .ret_from_sum_pNode
    ;Ended return node
    jmp .end_.if_stmt_1
.end_.if_stmt_1:
    ;Return node
    ;Ptr field access
    lea rax, [rbp - 8]
    mov rax, [rax]
    add rax, 0 
    mov eax, [rax]
    ;Ptr field access end
    ;Preparing to call function
    ;Saving used registers
    push rax
    ;Ptr field access
    lea rax, [rbp - 8]
    mov rax, [rax]
    add rax, 4 
    mov rax, [rax]
    ;Ptr field access end
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    ;Ensuring rsp is 16 byte aligned
    sub rsp, 8
    call _sum_pNode
    ;RAX is busy, move the returned value somewhere else AND THAT HAS NOT BEEN PUSHED
    mov ebx, eax
    ;Restoring rsp
    add rsp, 8
    pop rax
    ;Ended func call node
    add eax, ebx
    jmp .ret_from_sum_pNode
    ;Ended return node
.ret_from_sum_pNode:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_demo1:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    ;Done loading parameters
    ;Reassignment
    ;Field access
    lea rax, [rbp - 12]
    add rax, 0 
    ;Field access end
    mov ebx, 20
    mov dword [rax], ebx 
    ;Reassignment
    ;Field access
    lea rax, [rbp - 24]
    add rax, 0 
    ;Field access end
    mov ebx, 10
    mov dword [rax], ebx 
    ;Reassignment
    ;Field access
    lea rax, [rbp - 24]
    add rax, 4 
    ;Field access end
    ;Getting the address
    lea rbx, [rbp - 12]
    mov [rax], rbx 
    ;Preparing to call function
    mov rax, Lstr0
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_s
    ;Ended func call node
    ;Preparing to call function
    ;Preparing to call function
    ;Getting the address
    lea rax, [rbp - 24]
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _sum_pNode
    ;Ended func call node
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _print_u32
    ;Ended func call node
.ret_from_demo1:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_demo2:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    ;Loading parameters
    ;Done loading parameters
    ;Building assignment expression 
    ;Array init
    mov eax, 1
    mov [rbp - 40], eax
    mov eax, 2
    mov [rbp - 36], eax
    mov eax, 3
    mov [rbp - 32], eax
    mov eax, 4
    mov [rbp - 28], eax
    mov eax, 5
    mov [rbp - 24], eax
    ;Array init end
    ;Building assignment expression 
    mov eax, 0
    mov [rbp - 12], eax
    ;Lowered foreach node
    ;Building assignment expression 
    mov eax, 0
    mov [rbp - 20], eax
    ;While loop
.while_loop_2:
    ;While loop condition check
    mov eax, [rbp - 20]
    mov bl, 5
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.while_loop_2
    ;Body
    ;Reassignment
    lea rax, [rbp - 16]
    ;Array subscript
    lea rbx, [rbp - 40]
    ;Computing the index
    mov ecx, [rbp - 20]
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 4 
    add rbx, rcx 
    mov ebx, [rbx]
    ;Array subscript end
    mov dword [rax], ebx 
    ;Reassignment
    lea rax, [rbp - 12]
    mov ebx, [rbp - 16]
    add [rax], ebx 
    lea rax, [rbp - 20]
    mov ebx, [rax]
    inc dword [rax] 
    ;Back to while loop condition check
    jmp .while_loop_2

.end_.while_loop_2:
    ;End lowered foreach node
    ;Preparing to call function
    mov rax, Lstr1
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_s
    ;Ended func call node
    ;Preparing to call function
    mov eax, [rbp - 12]
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _print_u32
    ;Ended func call node
.ret_from_demo2:
    ;Restore the stack pointer and return
    add rsp, 40
    pop rbp
    ret
_fib_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], dil
    ;Done loading parameters
    ;If statement
.if_stmt_2:
    ;Condition check of if block
    mov al, [rbp - 8]
    mov bl, 1
    cmp al, bl
    setle al
    cmp al, 0
    je .end_.if_stmt_2
    ;Return node
    mov al, [rbp - 8]
    jmp .ret_from_fib_u8
    ;Ended return node
    jmp .end_.if_stmt_2
.end_.if_stmt_2:
    ;Return node
    ;Preparing to call function
    mov al, [rbp - 8]
    mov bl, 1
    sub al , bl
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _fib_u8
    ;Ended func call node
    ;Preparing to call function
    ;Saving used registers
    push rax
    mov al, [rbp - 8]
    mov bl, 2
    sub al , bl
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    ;Ensuring rsp is 16 byte aligned
    sub rsp, 8
    call _fib_u8
    ;RAX is busy, move the returned value somewhere else AND THAT HAS NOT BEEN PUSHED
    mov ebx, eax
    ;Restoring rsp
    add rsp, 8
    ;Collecting spilled registers
    pop rax
    ;Ended func call node
    add eax, ebx
    jmp .ret_from_fib_u8
    ;Ended return node
.ret_from_fib_u8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_demo3:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    ;Done loading parameters
    ;Building assignment expression 
    ;Preparing to call function
    mov al, 40
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _fib_u8
    ;Collecting spilled registers
    ;Ended func call node
    mov [rbp - 8], eax
    ;Preparing to call function
    mov rax, Lstr2
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_s
    ;Collecting spilled registers
    ;Ended func call node
    ;Preparing to call function
    mov eax, [rbp - 8]
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _print_u32
    ;Collecting spilled registers
    ;Ended func call node
.ret_from_demo3:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_main:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    ;Done loading parameters
    ;Preparing to call function
    call _demo1
    ;Collecting spilled registers
    ;Ended func call node
.ret_from_main:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
