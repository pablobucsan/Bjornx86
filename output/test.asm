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

_test_pA:
    push rbp
    mov rbp, rsp
    ;Align to 16 bytes
    sub rsp, 8
    ;Loading parameters
    mov qword [rbp - 8], rdi 
    ;Ptr field access
    lea rax, [rbp - 8]
    mov rax, [rax] 
    add rax, 0 
    ;Ptr field access end
    mov ebx, [rax] 
    inc dword [rax] 
    ;Restore the stack pointer and return
.ret_from_test_pA:
    add rsp, 8
    pop rbp
    ret

_main:
    push rbp
    mov rbp, rsp
    ;Align to 16 bytes
    sub rsp, 136
    ;Loading parameters
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 136]
    add rax, 0 
    ;Field access end
    ;Rvalue
    mov ebx, 0
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 136]
    add rax, 4 
    ;Field access end
    ;Rvalue
    mov ebx, 1
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 136]
    add rax, 8 
    ;Field access end
    ;Rvalue
    mov ebx, 2
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 116]
    add rax, 0 
    ;Field access end
    ;Rvalue
    mov ebx, 5
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 116]
    add rax, 4 
    ;Field access end
    ;Rvalue
    mov ebx, 1
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 116]
    add rax, 8 
    ;Field access end
    ;Rvalue
    mov ebx, 2
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 104]
    add rax, 0 
    ;Field access end
    ;Rvalue
    mov ebx, 7
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 104]
    add rax, 4 
    ;Field access end
    ;Rvalue
    mov ebx, 1
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    ;Field access
    lea rax, [rbp - 104]
    add rax, 8 
    ;Field access end
    ;Rvalue
    mov ebx, 2
    mov dword [rax], ebx 
    ;Reassignment
    ;LValue
    lea rax, [rbp - 136]
    ;RValue (address)
    lea rbx, [rbp - 116]
    ;Copying memory
    lea rdi, [rax]
    lea rsi, [rbx]
    mov rcx, 12 
    rep movsb
    ;Array init
    ;Getting the address
    lea rax, [rbp - 136]
    mov qword [rbp - 72], rax 
    ;Getting the address
    lea rax, [rbp - 116]
    mov qword [rbp - 64], rax 
    ;Getting the address
    lea rax, [rbp - 104]
    mov qword [rbp - 56], rax 
    ;Array init end
    ;Array init
    ;Copying
    lea rdi, [rbp - 48]
    add rdi, 0 
    lea rax, [rbp - 136]
    lea rsi, [rax]
    mov rcx, 12 
    rep movsb 
    ;Copying
    lea rdi, [rbp - 48]
    add rdi, 12 
    lea rax, [rbp - 116]
    lea rsi, [rax]
    mov rcx, 12 
    rep movsb 
    ;Copying
    lea rdi, [rbp - 48]
    add rdi, 24 
    lea rax, [rbp - 104]
    lea rsi, [rax]
    mov rcx, 12 
    rep movsb 
    ;Array init end
    ;Preparing to call function
    ;Field access
    ;Array subscript
    ;Computing the base address
    lea rax, [rbp - 48]
    ;Computing the index
    mov rbx, 0
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 12 
    add rax, rbx 
    ;Array subscript end
    add rax, 0 
    mov edi,[rax] 
    ;Field access end
    call _print_u32
    ;Ended func call ndoe
    ;Preparing to call function
    ;Field access
    ;Array subscript
    ;Computing the base address
    lea rax, [rbp - 48]
    ;Computing the index
    mov rbx, 1
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 12 
    add rax, rbx 
    ;Array subscript end
    add rax, 0 
    mov edi,[rax] 
    ;Field access end
    call _print_u32
    ;Ended func call ndoe
    ;Preparing to call function
    ;Field access
    ;Array subscript
    ;Computing the base address
    lea rax, [rbp - 48]
    ;Computing the index
    mov rbx, 2
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 12 
    add rax, rbx 
    ;Array subscript end
    add rax, 0 
    mov edi,[rax] 
    ;Field access end
    call _print_u32
    ;Ended func call ndoe
    ;Reassignment
    ;LValue
    lea rax, [rbp - 136]
    ;RValue (address)
    ;Array subscript
    ;Computing the base address
    lea rbx, [rbp - 48]
    ;Computing the index
    mov rcx, 2
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 12 
    add rbx, rcx 
    ;Array subscript end
    ;Copying memory
    lea rdi, [rax]
    lea rsi, [rbx]
    mov rcx, 12 
    rep movsb
    ;Preparing to call function
    ;Array subscript
    ;Computing the base address
    lea rax, [rbp - 72]
    ;Computing the index
    mov rbx, 0
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 8 
    add rax, rbx 
    mov rdi, [rax]
    ;Array subscript end
    call _test_pA
    ;Ended func call ndoe
    ;Preparing to call function
    ;Field access
    lea rax, [rbp - 136]
    add rax, 0 
    mov edi,[rax] 
    ;Field access end
    call _print_u32
    ;Ended func call ndoe
    ;Restore the stack pointer and return
.ret_from_main:
    add rsp, 136
    pop rbp
    ret

