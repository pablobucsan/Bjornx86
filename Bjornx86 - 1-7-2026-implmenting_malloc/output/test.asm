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
PROT_NONE dq 0
PROT_READ dq 1
PROT_WRITE dq 2
PROT_EXEC dq 4
MAP_SHARED dq 1
MAP_PRIVATE dq 2
MAP_ANONYMOUS dq 32
__arena_init db 0
__arenaHead dq 0



Lstr0: db 48,98,0
Lstr1: db 48,98,0
Lstr2: db 48,98,0
Lstr3: db 48,120,0
Lstr4: db 48,120,0
Lstr5: db 48,120,0
Lstr6: db 48,120,0
Lstr7: db 66,108,111,99,107,32,115,116,97,114,116,115,32,97,116,58,32,0
Lstr8: db 10,87,105,116,104,32,116,111,116,97,108,32,115,105,122,101,32,111,102,58,32,0
Lstr9: db 65,114,101,110,97,32,104,97,115,32,98,101,101,110,32,105,110,105,116,105,97,108,105,122,101,100,10,0
section .text

; Extern functions 
extern _putc_c
extern _write_u32_pc_u32
extern _open_pc_u32_u32
extern _close_u32
extern _mmap_p_u64_u64_u64_u64_u64

global _start
_start:
    mov rdi, qword [rsp]
    lea rsi, [rsp + 8]
    call _main_u64_ppc

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
    ;param
    mov al, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
ret_from_print_c:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 24], rdi
    ;Done loading parameters
    ;Building assignment expression 
    mov eax, 0
    mov [rbp - 16], eax
while_loop_0:
    ;While loop condition check
    ;Array subscript
    lea rax, [rbp - 24]
    ;PTR/STR subscript
    mov rax, [rax]
    ;Computing the index
    mov ebx, [rbp - 16]
    ;Computing the position of the element to retrieve
    imul ebx, ebx, 1 
    ;Finished promoting bigger size
    add rax, rbx
    mov al, [rax]
    ;Array subscript end
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je end_while_loop_0
    ;Body
    ;Preparing to call function
    ;param
    ;Array subscript
    lea rax, [rbp - 24]
    ;PTR/STR subscript
    mov rax, [rax]
    ;Computing the index
    lea rbx, [rbp - 16]
    mov ecx, [rbx]
    inc dword [rbx] 
    ;Computing the position of the element to retrieve
    imul ecx, ecx, 1 
    ;Finished promoting bigger size
    add rax, rcx
    mov al, [rax]
    ;Array subscript end
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Back to while loop condition check
    jmp while_loop_0

end_while_loop_0:
ret_from_print_pc:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_print_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 16], dil
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 2
    mov [rbp - 17], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 18], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 19], al
while_loop_1:
    ;While loop condition check
    mov al, [rbp - 17]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_while_loop_1
    ;Body
if_stmt_0:
    ;Condition check of if block
    mov al, [rbp - 16]
    mov bl, 10
    cmp al, bl
    setb al
    mov bl, [rbp - 19]
    test bl, bl
    setz cl
    test al, al
    setne al
    test cl, cl
    setne cl
    and al, cl
    cmp al, 0
    je end_if_stmt_0
    ;Reassignment
    lea rax, [rbp - 18]
    mov bl, [rbp - 17]
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 19]
    mov bl, 1
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    jmp end_if_stmt_0
end_if_stmt_0:
    ;Building assignment expression 
    mov al, [rbp - 16]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov [rbp - 20], dl
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    lea rbx, [rbp - 17]
    mov cl, [rbx]
    dec byte [rbx] 
    movsx rcx, cl
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 1 
    add rax, rcx
    ;Array subscript end
    mov bl, [rbp - 20]
    mov cl, 48
    add bl, cl
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 16]
    mov bl, [rbp - 16]
    mov cl, 10
    ;Spilling to stack
    push rax
    movzx eax, bl
    movzx ebx, cl
    xor edx, edx
    div ebx
    ;Collecting
    mov bl, al
    pop rax
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Back to while loop condition check
    jmp while_loop_1

end_while_loop_1:
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, 3
    movzx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    mov bl, 0
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Preparing to call function
    ;param
    ;Getting the address
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, [rbp - 18]
    movsx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
ret_from_print_u8:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_print_i8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], dil
    ;Done loading parameters
if_stmt_1:
    ;Condition check of if block
    mov al, [rbp - 8]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je end_if_stmt_1
    ;Preparing to call function
    ;param
    mov al, 45
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Reassignment
    lea rax, [rbp - 8]
    mov bl, [rbp - 8]
    neg bl
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    jmp end_if_stmt_1
end_if_stmt_1:
    ;Preparing to call function
    ;param
    mov al, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
ret_from_print_i8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_u16:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 18], di
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 4
    mov [rbp - 13], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 14], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 15], al
while_loop_2:
    ;While loop condition check
    mov al, [rbp - 13]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_while_loop_2
    ;Body
if_stmt_2:
    ;Condition check of if block
    mov ax, [rbp - 18]
    mov bl, 10
    movzx bx, bl
    ;Finished promoting bigger size
    cmp ax, bx
    setb al
    mov bl, [rbp - 15]
    test bl, bl
    setz cl
    test al, al
    setne al
    test cl, cl
    setne cl
    and al, cl
    cmp al, 0
    je end_if_stmt_2
    ;Reassignment
    lea rax, [rbp - 14]
    mov bl, [rbp - 13]
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 15]
    mov bl, 1
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    jmp end_if_stmt_2
end_if_stmt_2:
    ;Building assignment expression 
    mov ax, [rbp - 18]
    mov bl, 10
    movzx bx, bl
    ;Finished promoting bigger size
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    mov [rbp - 16], dl
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    lea rbx, [rbp - 13]
    mov cl, [rbx]
    dec byte [rbx] 
    movsx rcx, cl
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 1 
    add rax, rcx
    ;Array subscript end
    mov bl, [rbp - 16]
    mov cl, 48
    add bl, cl
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 18]
    mov bx, [rbp - 18]
    mov cx, 10
    ;Spilling to stack
    push rax
    movzx eax, bx
    movzx ebx, cx
    xor edx, edx
    div ebx
    ;Collecting
    mov bx, ax
    pop rax
    ;Reassign move instruction
    ;With size = 2
    mov word [rax], bx 
    ;Back to while loop condition check
    jmp while_loop_2

end_while_loop_2:
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, 5
    movzx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    mov bl, 0
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Preparing to call function
    ;param
    ;Getting the address
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, [rbp - 14]
    movsx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
ret_from_print_u16:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_print_i16:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], di
    ;Done loading parameters
if_stmt_3:
    ;Condition check of if block
    mov ax, [rbp - 8]
    mov bl, 0
    movzx bx, bl
    ;Finished promoting bigger size
    cmp ax, bx
    setl al
    cmp al, 0
    je end_if_stmt_3
    ;Preparing to call function
    ;param
    mov al, 45
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Reassignment
    lea rax, [rbp - 8]
    mov bx, [rbp - 8]
    neg bx
    ;Reassign move instruction
    ;With size = 2
    mov word [rax], bx 
    jmp end_if_stmt_3
end_if_stmt_3:
    ;Preparing to call function
    ;param
    mov ax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: ax
    mov di, ax
    call _print_u16
    ;Ended func call node
ret_from_print_i16:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 12], edi
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 9
    mov [rbp - 5], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 6], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 7], al
while_loop_3:
    ;While loop condition check
    mov al, [rbp - 5]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_while_loop_3
    ;Body
if_stmt_4:
    ;Condition check of if block
    mov eax, [rbp - 12]
    mov bl, 10
    movzx ebx, bl
    ;Finished promoting bigger size
    cmp eax, ebx
    setb al
    mov bl, [rbp - 7]
    test bl, bl
    setz cl
    test al, al
    setne al
    test cl, cl
    setne cl
    and al, cl
    cmp al, 0
    je end_if_stmt_4
    ;Reassignment
    lea rax, [rbp - 6]
    mov bl, [rbp - 5]
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 7]
    mov bl, 1
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    jmp end_if_stmt_4
end_if_stmt_4:
    ;Building assignment expression 
    mov eax, [rbp - 12]
    mov bl, 10
    movzx ebx, bl
    ;Finished promoting bigger size
    xor edx, edx
    div ebx
    mov [rbp - 8], dl
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    lea rbx, [rbp - 5]
    mov cl, [rbx]
    dec byte [rbx] 
    movsx rcx, cl
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 1 
    add rax, rcx
    ;Array subscript end
    mov bl, [rbp - 8]
    mov cl, 48
    add bl, cl
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 12]
    mov ebx, [rbp - 12]
    mov ecx, 10
    ;Spilling to stack
    push rax
    mov eax, ebx
    mov ebx, ecx
    xor edx, edx
    div ebx
    ;Collecting
    mov ebx, eax
    pop rax
    ;Reassign move instruction
    ;With size = 4
    mov dword [rax], ebx 
    ;Back to while loop condition check
    jmp while_loop_3

end_while_loop_3:
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, 10
    movzx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    mov bl, 0
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Preparing to call function
    ;param
    ;Getting the address
    ;Array subscript
    lea rax, [rbp - 24]
    ;Computing the index
    mov bl, [rbp - 6]
    movsx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
ret_from_print_u32:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_print_i32:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], edi
    ;Done loading parameters
if_stmt_5:
    ;Condition check of if block
    mov eax, [rbp - 8]
    mov bl, 0
    movzx ebx, bl
    ;Finished promoting bigger size
    cmp eax, ebx
    setl al
    cmp al, 0
    je end_if_stmt_5
    ;Preparing to call function
    ;param
    mov al, 45
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Reassignment
    lea rax, [rbp - 8]
    mov ebx, [rbp - 8]
    neg ebx
    ;Reassign move instruction
    ;With size = 4
    mov dword [rax], ebx 
    jmp end_if_stmt_5
end_if_stmt_5:
    ;Preparing to call function
    ;param
    mov eax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _print_u32
    ;Ended func call node
ret_from_print_i32:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    ;Loading parameters
    mov [rbp - 16], rdi
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 19
    mov [rbp - 5], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 6], al
    ;Building assignment expression 
    mov al, 0
    mov [rbp - 7], al
while_loop_4:
    ;While loop condition check
    mov al, [rbp - 5]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_while_loop_4
    ;Body
if_stmt_6:
    ;Condition check of if block
    mov rax, [rbp - 16]
    mov bl, 10
    movzx rbx, bl
    ;Finished promoting bigger size
    cmp rax, rbx
    setb al
    mov bl, [rbp - 7]
    test bl, bl
    setz cl
    test al, al
    setne al
    test cl, cl
    setne cl
    and al, cl
    cmp al, 0
    je end_if_stmt_6
    ;Reassignment
    lea rax, [rbp - 6]
    mov bl, [rbp - 5]
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 7]
    mov bl, 1
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    jmp end_if_stmt_6
end_if_stmt_6:
    ;Building assignment expression 
    mov rax, [rbp - 16]
    mov bl, 10
    movzx rbx, bl
    ;Finished promoting bigger size
    xor rdx, rdx
    div rbx
    mov [rbp - 8], dl
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 40]
    ;Computing the index
    lea rbx, [rbp - 5]
    mov cl, [rbx]
    dec byte [rbx] 
    movsx rcx, cl
    ;Computing the position of the element to retrieve
    imul rcx, rcx, 1 
    add rax, rcx
    ;Array subscript end
    mov bl, [rbp - 8]
    mov cl, 48
    add bl, cl
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Reassignment
    lea rax, [rbp - 16]
    mov rbx, [rbp - 16]
    mov rcx, 10
    ;Spilling to stack
    push rax
    mov rax, rbx
    mov rbx, rcx
    xor rdx, rdx
    div rbx
    ;Collecting
    mov rbx, rax
    pop rax
    ;Reassign move instruction
    ;With size = 8
    mov qword [rax], rbx 
    ;Back to while loop condition check
    jmp while_loop_4

end_while_loop_4:
    ;Reassignment
    ;Array subscript
    lea rax, [rbp - 40]
    ;Computing the index
    mov bl, 20
    movzx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    mov bl, 0
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Preparing to call function
    ;param
    ;Getting the address
    ;Array subscript
    lea rax, [rbp - 40]
    ;Computing the index
    mov bl, [rbp - 6]
    movsx rbx, bl
    ;Computing the position of the element to retrieve
    imul rbx, rbx, 1 
    add rax, rbx
    ;Array subscript end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
ret_from_print_u64:
    ;Restore the stack pointer and return
    add rsp, 40
    pop rbp
    ret
_print_i64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
if_stmt_7:
    ;Condition check of if block
    mov rax, [rbp - 8]
    mov bl, 0
    movzx rbx, bl
    ;Finished promoting bigger size
    cmp rax, rbx
    setl al
    cmp al, 0
    je end_if_stmt_7
    ;Preparing to call function
    ;param
    mov al, 45
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
    ;Reassignment
    lea rax, [rbp - 8]
    mov rbx, [rbp - 8]
    neg rbx
    ;Reassign move instruction
    ;With size = 8
    mov qword [rax], rbx 
    jmp end_if_stmt_7
end_if_stmt_7:
    ;Preparing to call function
    ;param
    mov rax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_u64
    ;Ended func call node
ret_from_print_i64:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printBinary_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 6], dil
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 1
    mov [rbp - 7], al
    ;Preparing to call function
    ;param
    mov rax, Lstr0
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 7
    mov [rbp - 8], al
for_loop_0:
    ;Condition check of for loop
    mov al, [rbp - 8]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_0
    ;Body
    ;Preparing to call function
    ;param
    mov al, [rbp - 6]
    mov bl, [rbp - 8]
    mov cl, bl
    shr al, cl
    mov bl, [rbp - 7]
    and al,bl
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 8]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_0

end_for_loop_0:
ret_from_printBinary_u8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printBinary_u16:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 6], di
    ;Done loading parameters
    ;Building assignment expression 
    mov ax, 1
    mov [rbp - 8], ax
    ;Preparing to call function
    ;param
    mov rax, Lstr1
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 4], al
for_loop_1:
    ;Condition check of for loop
    mov al, [rbp - 4]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_1
    ;Body
    ;Preparing to call function
    ;param
    mov ax, [rbp - 6]
    mov bl, [rbp - 4]
    movsx bx, bl
    ;Finished promoting bigger size
    mov cl, bl
    shr ax, cl
    mov bx, [rbp - 8]
    and ax,bx
    ;param finished
    ;Changing to FPR, par was stored in GPR: ax
    mov di, ax
    call _print_u16
    ;Ended func call node
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 4]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_1

end_for_loop_1:
ret_from_printBinary_u16:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printBinary_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 20], edi
    ;Done loading parameters
    ;Building assignment expression 
    mov eax, 1
    mov [rbp - 24], eax
    ;Preparing to call function
    ;param
    mov rax, Lstr2
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 31
    mov [rbp - 16], al
for_loop_2:
    ;Condition check of for loop
    mov al, [rbp - 16]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_2
    ;Body
    ;Preparing to call function
    ;param
    mov eax, [rbp - 20]
    mov bl, [rbp - 16]
    movsx ebx, bl
    ;Finished promoting bigger size
    mov cl, bl
    shr eax, cl
    mov ebx, [rbp - 24]
    and eax,ebx
    ;param finished
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _print_u32
    ;Ended func call node
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 16]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_2

end_for_loop_2:
ret_from_printBinary_u32:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_printBinary_i8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], dil
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov al, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _printBinary_u8
    ;Ended func call node
ret_from_printBinary_i8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printBinary_i16:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], di
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov ax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: ax
    mov di, ax
    call _printBinary_u16
    ;Ended func call node
ret_from_printBinary_i16:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printBinary_i32:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], edi
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov eax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _printBinary_u32
    ;Ended func call node
ret_from_printBinary_i32:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 4], dil
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 5], al
    ;Preparing to call function
    ;param
    mov rax, Lstr3
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 1
    mov [rbp - 6], al
for_loop_3:
    ;Condition check of for loop
    mov al, [rbp - 6]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_3
    ;Body
    ;Building assignment expression 
    mov al, 4
    mov bl, [rbp - 6]
    imul bl 
    mov bl, [rbp - 4]
    movzx bx, bl
    ;Finished promoting bigger size
    mov cl, al
    shr bx, cl
    mov al, [rbp - 5]
    movzx ax, al
    ;Finished promoting bigger size
    and bx,ax
    mov [rbp - 7], bl
if_stmt_8:
    ;Condition check of if block
    mov al, [rbp - 7]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je else_stmt_0
    ;Building assignment expression 
    mov al, [rbp - 7]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    mov [rbp - 8], bl
    ;Preparing to call function
    ;param
    mov al, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_c
    ;Ended func call node
    jmp end_if_stmt_8
else_stmt_0:
    ;Preparing to call function
    ;param
    mov al, [rbp - 7]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
end_if_stmt_8:
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 6]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_3

end_for_loop_3:
ret_from_printHex_u8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_u16:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], di
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 3], al
    ;Preparing to call function
    ;param
    mov rax, Lstr4
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 3
    mov [rbp - 4], al
for_loop_4:
    ;Condition check of for loop
    mov al, [rbp - 4]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_4
    ;Body
    ;Building assignment expression 
    mov al, 4
    mov bl, [rbp - 4]
    imul bl 
    mov bx, [rbp - 8]
    mov cl, al
    shr bx, cl
    mov al, [rbp - 3]
    movzx ax, al
    ;Finished promoting bigger size
    and bx,ax
    mov [rbp - 5], bl
if_stmt_9:
    ;Condition check of if block
    mov al, [rbp - 5]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je else_stmt_1
    ;Building assignment expression 
    mov al, [rbp - 5]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    mov [rbp - 6], bl
    ;Preparing to call function
    ;param
    mov al, [rbp - 6]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_c
    ;Ended func call node
    jmp end_if_stmt_9
else_stmt_1:
    ;Preparing to call function
    ;param
    mov al, [rbp - 5]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
end_if_stmt_9:
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 4]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_4

end_for_loop_4:
ret_from_printHex_u16:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], edi
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 1], al
    ;Preparing to call function
    ;param
    mov rax, Lstr5
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 7
    mov [rbp - 2], al
for_loop_5:
    ;Condition check of for loop
    mov al, [rbp - 2]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_5
    ;Body
    ;Building assignment expression 
    mov al, 4
    mov bl, [rbp - 2]
    imul bl 
    mov ebx, [rbp - 8]
    movsx eax, ax
    ;Finished promoting bigger size
    mov cl, al
    shr ebx, cl
    mov al, [rbp - 1]
    movzx eax, al
    ;Finished promoting bigger size
    and ebx,eax
    mov [rbp - 3], bl
if_stmt_10:
    ;Condition check of if block
    mov al, [rbp - 3]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je else_stmt_2
    ;Building assignment expression 
    mov al, [rbp - 3]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    mov [rbp - 4], bl
    ;Preparing to call function
    ;param
    mov al, [rbp - 4]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_c
    ;Ended func call node
    jmp end_if_stmt_10
else_stmt_2:
    ;Preparing to call function
    ;param
    mov al, [rbp - 3]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
end_if_stmt_10:
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 2]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_5

end_for_loop_5:
ret_from_printHex_u32:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 24], rdi
    ;Done loading parameters
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 13], al
    ;Preparing to call function
    ;param
    mov rax, Lstr6
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Initial assignment of for loop iterator
    ;Building assignment expression 
    mov al, 15
    mov [rbp - 14], al
for_loop_6:
    ;Condition check of for loop
    mov al, [rbp - 14]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je end_for_loop_6
    ;Body
    ;Building assignment expression 
    mov al, 4
    mov bl, [rbp - 14]
    imul bl 
    mov rbx, [rbp - 24]
    movsx rax, ax
    ;Finished promoting bigger size
    mov cl, al
    shr rbx, cl
    mov al, [rbp - 13]
    movzx rax, al
    ;Finished promoting bigger size
    and rbx,rax
    mov [rbp - 15], bl
if_stmt_11:
    ;Condition check of if block
    mov al, [rbp - 15]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je else_stmt_3
    ;Building assignment expression 
    mov al, [rbp - 15]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    mov [rbp - 16], bl
    ;Preparing to call function
    ;param
    mov al, [rbp - 16]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_c
    ;Ended func call node
    jmp end_if_stmt_11
else_stmt_3:
    ;Preparing to call function
    ;param
    mov al, [rbp - 15]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _print_u8
    ;Ended func call node
end_if_stmt_11:
    ;Reassignment of the for loop iterator
    lea rax, [rbp - 14]
    mov bl, [rax]
    dec byte [rax] 
    ;Back to condition check of for loop
    jmp for_loop_6

end_for_loop_6:
ret_from_printHex_u64:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_printHex_i8:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], dil
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov al, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _printHex_u8
    ;Ended func call node
ret_from_printHex_i8:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_i16:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], di
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov ax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: ax
    mov di, ax
    call _printHex_u16
    ;Ended func call node
ret_from_printHex_i16:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_i32:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], edi
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov eax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: eax
    mov edi, eax
    call _printHex_u32
    ;Ended func call node
ret_from_printHex_i32:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_printHex_i64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov rax, [rbp - 8]
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _printHex_u64
    ;Ended func call node
ret_from_printHex_i64:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_print_p:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    ;Loading parameters
    mov [rbp - 16], rdi
    ;Done loading parameters
    ;Building assignment expression 
    mov rax, [rbp - 16]
    mov [rbp - 24], rax
    ;Preparing to call function
    ;param
    mov rax, [rbp - 24]
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _printHex_u64
    ;Ended func call node
ret_from_print_p:
    ;Restore the stack pointer and return
    add rsp, 24
    pop rbp
    ret
_displayBlock_pArenaFreeBlock__:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
    ;Preparing to call function
    ;param
    mov rax, Lstr7
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Preparing to call function
    ;param
    ;Ptr field access
    lea rax, [rbp - 8]
    mov rax, [rax]
    add rax, 0 
    mov rax, [rax]
    ;Ptr field access end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_p
    ;Ended func call node
    ;Preparing to call function
    ;param
    mov rax, Lstr8
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Preparing to call function
    ;param
    ;Ptr field access
    lea rax, [rbp - 8]
    mov rax, [rax]
    add rax, 16 
    mov rax, [rax]
    ;Ptr field access end
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_u64
    ;Ended func call node
    ;Preparing to call function
    ;param
    mov al, 10
    ;param finished
    ;Changing to FPR, par was stored in GPR: al
    mov dil, al
    call _putc_c
    ;Ended func call node
ret_from_displayBlock_pArenaFreeBlock__:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
___initArena:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    ;Done loading parameters
    ;Reassignment
    lea rax, [__arenaHead]
    ;Spilling to stack
    push rax
    ;Preparing to call function
    ;param
    mov rax, [null]
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    ;param
    mov rax, 24
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rsi, rax
    ;param
    mov rax, [PROT_READ]
    mov rbx, [PROT_WRITE]
    or rax,rbx
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdx, rax
    ;param
    mov rax, [MAP_PRIVATE]
    mov rbx, [MAP_ANONYMOUS]
    or rax,rbx
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rcx, rax
    ;param
    mov rax, 1
    neg rax
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov r8, rax
    ;param
    mov rax, 0
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov r9, rax
    ;Ensuring rsp is 16 byte aligned
    sub rsp, 8
    call _mmap_p_u64_u64_u64_u64_u64
    ;Restoring rsp
    add rsp, 8
    ;Ended func call node
    ;Collecting
    mov rbx, rax
    pop rax
    mov [rax], rbx 
    ;Reassignment
    lea rax, [__arena_init]
    mov bl, 1
    ;Reassign move instruction
    ;With size = 1
    mov byte [rax], bl 
    ;Preparing to call function
    ;param
    mov rax, Lstr9
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _print_pc
    ;Ended func call node
    ;Reassignment
    ;Ptr field access
    lea rax, [__arenaHead]
    mov rax, [rax]
    add rax, 0 
    ;Ptr field access end
    mov rbx, [__arenaHead]
    mov [rax], rbx 
    ;Reassignment
    ;Ptr field access
    lea rax, [__arenaHead]
    mov rax, [rax]
    add rax, 8 
    ;Ptr field access end
    mov rbx, [null]
    mov [rax], rbx 
    ;Reassignment
    ;Ptr field access
    lea rax, [__arenaHead]
    mov rax, [rax]
    add rax, 16 
    ;Ptr field access end
    mov rbx, 24
    ;Reassign move instruction
    ;With size = 8
    mov qword [rax], rbx 
    ;Preparing to call function
    ;param
    mov rax, [__arenaHead]
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _displayBlock_pArenaFreeBlock__
    ;Ended func call node
ret_from___initArena:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_malloc_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    ;Loading parameters
    mov [rbp - 8], rdi
    ;Done loading parameters
if_stmt_12:
    ;Condition check of if block
    mov al, [__arena_init]
    mov bl, 0
    cmp al, bl
    sete al
    cmp al, 0
    je end_if_stmt_12
    ;Preparing to call function
    call ___initArena
    ;Ended func call node
    jmp end_if_stmt_12
end_if_stmt_12:
    ;Return node
    mov rax, [null]
    jmp ret_from_malloc_u64
    ;Ended return node
ret_from_malloc_u64:
    ;Restore the stack pointer and return
    add rsp, 8
    pop rbp
    ret
_main_u64_ppc:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    ;Loading parameters
    mov [rbp - 16], rdi
    mov [rbp - 24], rsi
    ;Done loading parameters
    ;Building assignment expression 
    ;Preparing to call function
    ;param
    mov rax, 1
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _malloc_u64
    ;Ended func call node
    mov [rbp - 32], rax
    ;Building assignment expression 
    ;Preparing to call function
    ;param
    mov rax, 1
    ;param finished
    ;Changing to FPR, par was stored in GPR: rax
    mov rdi, rax
    call _malloc_u64
    ;Ended func call node
    mov [rbp - 40], rax
ret_from_main_u64_ppc:
    ;Restore the stack pointer and return
    add rsp, 40
    pop rbp
    ret
