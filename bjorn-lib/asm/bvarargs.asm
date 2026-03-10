section .data
__TYPE_U8: db 0
__TYPE_U16: db 1
__TYPE_U32: db 2
__TYPE_U64: db 3
__TYPE_I8: db 4
__TYPE_I16: db 5
__TYPE_I32: db 6
__TYPE_I64: db 7
__TYPE_CHAR: db 8
__TYPE_STR: db 9
__TYPE_BOOL: db 10
__TYPE_PTR: db 11
null: dq 0
__PUSH_DECR_RSP_BY: dq 8
section .text
_varg_next_p:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rbx, qword [__PUSH_DECR_RSP_BY]
    sub rax , rbx
    jmp .ret_from_varg_next_p
.ret_from_varg_next_p:
    add rsp, 8
    pop rbp
    ret
_varg_count_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov rax, 0
    mov qword [rbp - 16], rax
    mov rax, 0
    mov qword [rbp - 24], rax
.for_loop_28:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_28
.if_stmt_48:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_48
    lea rax, [rbp - 16]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_48
.end_.if_stmt_48:
.r_for_loop_28:
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_28

.end_.for_loop_28:
    mov rax, qword [rbp - 16]
    jmp .ret_from_varg_count_pc
.ret_from_varg_count_pc:
    add rsp, 24
    pop rbp
    ret
_varg_create_pc_p:
    push rbp
    mov rbp, rsp
    sub rsp, 72
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
.if_stmt_49:
    mov rax, qword [rbp - 16]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_49
    mov rax, qword [null]
    jmp .ret_from_varg_create_pc_p
    jmp .end_.if_stmt_49
.end_.if_stmt_49:
    mov rax, 0
    mov qword [rbp - 24], rax
    mov rax, 0
    mov qword [rbp - 32], rax
    mov rax, 0
    mov qword [rbp - 40], rax
.for_loop_29:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_29
.if_stmt_50:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_50
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_50
.end_.if_stmt_50:
    lea rax, [rbp - 32]
    mov rbx, qword [rax]
    inc qword [rax] 
.r_for_loop_29:
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_29

.end_.for_loop_29:
.if_stmt_51:
    mov rax, qword [rbp - 24]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_51
    mov rax, qword [null]
    jmp .ret_from_varg_create_pc_p
    jmp .end_.if_stmt_51
.end_.if_stmt_51:
    mov rax, 32
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 48]
    mov qword [rbx], rax 
    mov rax, 0
    mov rbx, qword [rbp - 48]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 48]
    mov [rbx + 24], rax 
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rbx, 8
    imul rbx, rax
    mov rdi, rbx
    call _malloc_u64
    mov rbx, qword [rbp - 48]
    mov [rbx + 16], rax 
    mov rax, 0
    mov qword [rbp - 56], rax
.for_loop_30:
    mov rax, qword [rbp - 56]
    mov rbx, qword [rbp - 24]
    cmp rax, rbx
    setb al
    cmp al, 0
    je .end_.for_loop_30
    mov rax, 56
    mov rdi, rax
    call _malloc_u64
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 56]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov [rbx], rax 
.r_for_loop_30:
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_30

.end_.for_loop_30:
    mov rax, 0
    mov qword [rbp - 64], rax
    mov rax, 0
    mov qword [rbp - 72], rax
.while_loop_4:
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 32]
    cmp rax, rbx
    setb al
    test al, al
    setne al
    jz .skip_right_0
    mov rbx, qword [rbp - 72]
    mov rcx, qword [rbp - 24]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
.skip_right_0:
    and al, bl
    cmp al, 0
    je .end_.while_loop_4
.if_stmt_52:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_16
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_53:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_52
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_54:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_54
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_55:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    test al, al
    setne al
    jnz .skip_right_1
    lea rbx, [rbp - 8]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 64]
    add rbx, rcx
    mov bl, byte [rbx]
    mov cl, 120
    cmp bl, cl
    sete bl
    test bl, bl
    setne bl
.skip_right_1:
    or al, bl
    cmp al, 0
    je .elseif_stmt_50
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_U8]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov al, byte [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov byte [rbx + 8], al 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_50:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_55
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_I8]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov al, byte [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov byte [rbx + 8], al 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.end_.if_stmt_55:
    jmp .end_.if_stmt_54
.end_.if_stmt_54:
.if_stmt_56:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    test al, al
    setne al
    jnz .skip_right_2
    lea rbx, [rbp - 8]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 64]
    add rbx, rcx
    mov bl, byte [rbx]
    mov cl, 120
    cmp bl, cl
    sete bl
    test bl, bl
    setne bl
.skip_right_2:
    or al, bl
    cmp al, 0
    je .elseif_stmt_51
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_U16]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov ax, word [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov word [rbx + 8], ax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_56
.elseif_stmt_51:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_56
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_I16]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov ax, word [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov word [rbx + 8], ax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_56
.end_.if_stmt_56:
    jmp .end_.if_stmt_53
.elseif_stmt_52:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    test al, al
    setne al
    jnz .skip_right_3
    lea rbx, [rbp - 8]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 64]
    add rbx, rcx
    mov bl, byte [rbx]
    mov cl, 120
    cmp bl, cl
    sete bl
    test bl, bl
    setne bl
.skip_right_3:
    or al, bl
    cmp al, 0
    je .elseif_stmt_53
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_U32]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov eax, dword [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov dword [rbx + 8], eax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_53
.elseif_stmt_53:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_54
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_I32]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov eax, dword [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov dword [rbx + 8], eax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_53
.elseif_stmt_54:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_55
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_STR]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_53
.elseif_stmt_55:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 99
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_56
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_CHAR]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov al, byte [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov byte [rbx + 8], al 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_53
.elseif_stmt_56:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_57
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_BOOL]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov al, byte [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov byte [rbx + 8], al 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_53
.elseif_stmt_57:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_53
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_57:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    test al, al
    setne al
    jnz .skip_right_4
    lea rbx, [rbp - 8]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 64]
    add rbx, rcx
    mov bl, byte [rbx]
    mov cl, 120
    cmp bl, cl
    sete bl
    test bl, bl
    setne bl
.skip_right_4:
    or al, bl
    cmp al, 0
    je .elseif_stmt_58
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov al, byte [__TYPE_U64]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_57
.elseif_stmt_58:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 64]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_57
    mov al, byte [__TYPE_I64]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 72]
    imul rcx, rcx, 8 
    add rbx, rcx
    mov rbx, qword [rbx]
    mov byte [rbx], al 
    mov rax, qword [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 48]
    mov rbx, qword [rbx + 16]
    lea rcx, [rbp - 72]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    imul rdx, rdx, 8 
    add rbx, rdx
    mov rbx, qword [rbx]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 16]
    mov [rbx], rax 
    jmp .end_.if_stmt_57
.end_.if_stmt_57:
    jmp .end_.if_stmt_53
.end_.if_stmt_53:
    jmp .end_.if_stmt_52
.else_stmt_16:
    lea rax, [rbp - 64]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_52:
    jmp .while_loop_4

.end_.while_loop_4:
    mov rax, qword [rbp - 48]
    jmp .ret_from_varg_create_pc_p
.ret_from_varg_create_pc_p:
    add rsp, 72
    pop rbp
    ret
_varg_reset_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, 0
    mov rbx, qword [rbp - 8]
    mov qword [rbx + 8], rax 
.ret_from_varg_reset_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getu8_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov al, byte [rax + 8]
    jmp .ret_from_varg_getu8_pVA_list
.ret_from_varg_getu8_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getu16_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov ax, word [rax + 8]
    jmp .ret_from_varg_getu16_pVA_list
.ret_from_varg_getu16_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getu32_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov eax, dword [rax + 8]
    jmp .ret_from_varg_getu32_pVA_list
.ret_from_varg_getu32_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getu64_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov rax, qword [rax + 8]
    jmp .ret_from_varg_getu64_pVA_list
.ret_from_varg_getu64_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_geti8_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov al, byte [rax + 8]
    jmp .ret_from_varg_geti8_pVA_list
.ret_from_varg_geti8_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_geti16_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov ax, word [rax + 8]
    jmp .ret_from_varg_geti16_pVA_list
.ret_from_varg_geti16_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_geti32_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov eax, dword [rax + 8]
    jmp .ret_from_varg_geti32_pVA_list
.ret_from_varg_geti32_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_geti64_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov rax, qword [rax + 8]
    jmp .ret_from_varg_geti64_pVA_list
.ret_from_varg_geti64_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getchar_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov al, byte [rax + 8]
    jmp .ret_from_varg_getchar_pVA_list
.ret_from_varg_getchar_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getstr_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov rax, qword [rax + 8]
    jmp .ret_from_varg_getstr_pVA_list
.ret_from_varg_getstr_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getbool_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov al, byte [rax + 8]
    jmp .ret_from_varg_getbool_pVA_list
.ret_from_varg_getbool_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_getptr_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 8]
    mov rcx, qword [rbx + 8]
    inc qword [rbx + 8] 
    imul rcx, rcx, 8 
    add rax, rcx
    mov rax, qword [rax]
    mov rax, qword [rax + 8]
    jmp .ret_from_varg_getptr_pVA_list
.ret_from_varg_getptr_pVA_list:
    add rsp, 8
    pop rbp
    ret
_varg_free_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
.if_stmt_58:
    mov rax, qword [rbp - 16]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_58
    jmp .ret_from_varg_free_pVA_list
    jmp .end_.if_stmt_58
.end_.if_stmt_58:
    mov rax, 0
    mov qword [rbp - 24], rax
.for_loop_31:
    mov rax, qword [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    cmp rbx, rax
    setb bl
    cmp bl, 0
    je .end_.for_loop_31
    mov rax, qword [rbp - 16]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 24]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov rdi, rax
    call _free_p
.r_for_loop_31:
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_31

.end_.for_loop_31:
    mov rax, qword [rbp - 16]
    mov rax, qword [rax + 16]
    mov rdi, rax
    call _free_p
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _free_p
.ret_from_varg_free_pVA_list:
    add rsp, 24
    pop rbp
    ret
