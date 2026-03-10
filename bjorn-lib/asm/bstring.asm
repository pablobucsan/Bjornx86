section .data
Lstr_bstring_0: db 116,114,117,101,0
Lstr_bstring_1: db 102,97,108,115,101,0
Lstr_bstring_2: db 116,114,117,101,0
Lstr_bstring_3: db 102,97,108,115,101,0
O_RDONLY: dw 0 
O_WRONLY: dw 1 
O_RDWR: dw 2 
O_CREATE: dw 64 
O_TRUNC: dw 512 
O_APPEND: dw 1024 
READ: dw 0 
READ_UPDATE: dw 2 
WRITE: dw 577 
READ_WRITE: dw 578 
APPEND: dw 1089 
READ_APPEND: dw 1090 
USER_READ: dw 256 
USER_WRITE: dw 128 
USER_EXE: dw 64 
GROUP_READ: dw 32 
GROUP_WRITE: dw 16 
GROUP_EXE: dw 8 
OTHER_READ: dw 4 
OTHER_WRITE: dw 2 
OTHER_EXE: dw 1 
USER_RWX: dw 448 
GROUP_RWX: dw 56 
OTHER_RWX: dw 7 
MODE644: dw 420 
MOD755: dw 493 
MODE666: dw 438 
MODE777: dw 511 
EOF: db -1
null: dq 0
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
section .text
_strlen_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov rax, 0
    mov qword [rbp - 16], rax
    mov rax, 0
    mov qword [rbp - 24], rax
.for_loop_0:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_0
    lea rax, [rbp - 16]
    mov rbx, qword [rax]
    inc qword [rax] 
.r_for_loop_0:
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_0

.end_.for_loop_0:
    mov rax, qword [rbp - 16]
    jmp .ret_from_strlen_pc
.ret_from_strlen_pc:
    add rsp, 24
    pop rbp
    ret
_cmpstring_pc_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, 0
    mov qword [rbp - 24], rax
.while_loop_11:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    test al, al
    setne al
    jz .skip_right_0
    lea rbx, [rbp - 16]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 24]
    add rbx, rcx
    mov bl, byte [rbx]
    mov cl, 0
    cmp bl, cl
    setne bl
    test bl, bl
    setne bl
.skip_right_0:
    and al, bl
    cmp al, 0
    je .end_.while_loop_11
.if_stmt_31:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    lea rbx, [rbp - 16]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 24]
    add rbx, rcx
    mov bl, byte [rbx]
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.if_stmt_31
    mov al, 0
    jmp .ret_from_cmpstring_pc_pc
    jmp .end_.if_stmt_31
.end_.if_stmt_31:
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .while_loop_11

.end_.while_loop_11:
    lea rax, [rbp - 8]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    lea rbx, [rbp - 16]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 24]
    add rbx, rcx
    mov bl, byte [rbx]
    cmp al, bl
    sete al
    jmp .ret_from_cmpstring_pc_pc
.ret_from_cmpstring_pc_pc:
    add rsp, 24
    pop rbp
    ret
_strdup_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 16], rax
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 24], rax
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov rax, qword [rbp - 8]
    mov rsi, rax
    mov rax, qword [rbp - 16]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov al, 0
    lea rbx, [rbp - 24]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 16]
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 24]
    jmp .ret_from_strdup_pc
.ret_from_strdup_pc:
    add rsp, 24
    pop rbp
    ret
_strtolu_pc_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 56
    mov qword [rbp - 24], rdi
    mov byte [rbp - 16], sil
    mov rax, 0
    mov qword [rbp - 32], rax
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 40], rax
    mov rax, 1
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 40]
    mov rbx, 1
    sub rax , rbx
    mov qword [rbp - 56], rax
.for_loop_1:
    mov rax, qword [rbp - 56]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    seta al
    cmp al, 0
    je .end_.for_loop_1
    lea rax, [rbp - 24]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov rbx, 48
    movzx rax, al
    sub rax , rbx
    mov rbx, qword [rbp - 48]
    imul rax, rbx
    lea rbx, [rbp - 32]
    add [rbx], rax 
    mov rax, qword [rbp - 48]
    mov bl, byte [rbp - 16]
    movzx rbx, bl
    imul rax, rbx
    lea rbx, [rbp - 48]
    mov qword [rbx], rax 
.r_for_loop_1:
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    dec qword [rax] 
    jmp .for_loop_1

.end_.for_loop_1:
.if_stmt_32:
    lea rax, [rbp - 24]
    mov rax, qword [rax]
    mov bl, 0
    movzx rbx, bl
    add rax, rbx
    mov al, byte [rax]
    mov bl, 45
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_8
    mov rax, qword [rbp - 32]
    mov rbx, -1
    imul rax, rbx
    lea rbx, [rbp - 32]
    mov qword [rbx], rax 
    jmp .end_.if_stmt_32
.else_stmt_8:
    lea rax, [rbp - 24]
    mov rax, qword [rax]
    mov bl, 0
    movzx rbx, bl
    add rax, rbx
    mov al, byte [rax]
    mov rbx, 48
    movzx rax, al
    sub rax , rbx
    mov rbx, qword [rbp - 48]
    imul rax, rbx
    lea rbx, [rbp - 32]
    add [rbx], rax 
.end_.if_stmt_32:
    mov rax, qword [rbp - 32]
    jmp .ret_from_strtolu_pc_u8
.ret_from_strtolu_pc_u8:
    add rsp, 56
    pop rbp
    ret
_strcat_pc_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 56
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 32], rax
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 32]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov rbx, 1
    add rax, rbx
    mov rdi, rax
    call _zalloc_u64
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 48]
    mov qword [rbp - 56], rax
    mov rax, qword [rbp - 56]
    mov rdi, rax
    mov rax, qword [rbp - 16]
    mov rsi, rax
    mov rax, qword [rbp - 32]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, qword [rbp - 56]
    mov rbx, qword [rbp - 32]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    mov rax, qword [rbp - 40]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov al, 0
    lea rbx, [rbp - 48]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 32]
    mov rdx, qword [rbp - 40]
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 48]
    jmp .ret_from_strcat_pc_pc
.ret_from_strcat_pc_pc:
    add rsp, 56
    pop rbp
    ret
___needed_size_sprintf_pc_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 56
    mov qword [rbp - 24], rdi
    mov qword [rbp - 32], rsi
    mov rax, qword [rbp - 32]
    mov rax, qword [rax]
    push rax
    mov rax, qword [rbp - 24]
    mov rdi, rax
    sub rsp, 8
    call _strlen_pc
    add rsp, 8
    mov rbx, rax
    pop rax
    sub rbx , rax
    mov qword [rbp - 40], rbx
    mov rax, 0
    mov qword [rbp - 48], rax
.for_loop_2:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 48]
    cmp rbx, rax
    setb bl
    cmp bl, 0
    je .end_.for_loop_2
.if_stmt_33:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_U8]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_1
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getu8_pVA_list
    push rdi
    movzx rdi, al
    mov rax, 10
    mov rsi, rax
    sub rsp, 8
    call _digitsof_i64_u64
    add rsp, 8
    movzx rbx, al
    pop rdi
    mov rax, 3
    sub rbx , rax
    lea rax, [rbp - 40]
    add [rax], rbx 
    jmp .end_.if_stmt_33
.elseif_stmt_1:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_U16]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_2
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getu16_pVA_list
    push rdi
    movzx rdi, ax
    mov rax, 10
    mov rsi, rax
    sub rsp, 8
    call _digitsof_i64_u64
    add rsp, 8
    movzx rbx, al
    pop rdi
    mov rax, 2
    sub rbx , rax
    lea rax, [rbp - 40]
    add [rax], rbx 
    jmp .end_.if_stmt_33
.elseif_stmt_2:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_U32]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_3
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getu32_pVA_list
    push rdi
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    sub rsp, 8
    call _digitsof_i64_u64
    add rsp, 8
    movzx rbx, al
    pop rdi
    mov rax, 1
    sub rbx , rax
    lea rax, [rbp - 40]
    add [rax], rbx 
    jmp .end_.if_stmt_33
.elseif_stmt_3:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_U64]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_4
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getu64_pVA_list
    push rdi
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    sub rsp, 8
    call _digitsof_i64_u64
    add rsp, 8
    movzx rbx, al
    pop rdi
    mov rax, 2
    sub rbx , rax
    lea rax, [rbp - 40]
    add [rax], rbx 
    jmp .end_.if_stmt_33
.elseif_stmt_4:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_I8]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_5
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_geti8_pVA_list
    mov byte [rbp - 9], al
.if_stmt_34:
    mov al, byte [rbp - 9]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.if_stmt_34
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_34
.end_.if_stmt_34:
    mov al, byte [rbp - 9]
    movsx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov rbx, 3
    movzx rax, al
    sub rax , rbx
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.elseif_stmt_5:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_I16]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_6
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_geti16_pVA_list
    mov word [rbp - 12], ax
.if_stmt_35:
    mov ax, word [rbp - 12]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    setl al
    cmp al, 0
    je .end_.if_stmt_35
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_35
.end_.if_stmt_35:
    mov ax, word [rbp - 12]
    movsx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov rbx, 2
    movzx rax, al
    sub rax , rbx
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.elseif_stmt_6:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_I32]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_7
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_geti32_pVA_list
    mov dword [rbp - 16], eax
.if_stmt_36:
    mov eax, dword [rbp - 16]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_36
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_36
.end_.if_stmt_36:
    mov eax, dword [rbp - 16]
    movsxd rdi, eax 
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov rbx, 1
    movzx rax, al
    sub rax , rbx
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.elseif_stmt_7:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_I64]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_8
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_geti64_pVA_list
    mov qword [rbp - 56], rax
.if_stmt_37:
    mov rax, qword [rbp - 56]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_37
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_37
.end_.if_stmt_37:
    mov rax, qword [rbp - 56]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov rbx, 2
    movzx rax, al
    sub rax , rbx
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.elseif_stmt_8:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_CHAR]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_9
    mov rax, 0
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.elseif_stmt_9:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_STR]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_10
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getstr_pVA_list
    push rdi
    mov rdi, rax
    sub rsp, 8
    call _strlen_pc
    add rsp, 8
    mov rbx, rax
    pop rdi
    mov rax, 1
    sub rbx , rax
    lea rax, [rbp - 40]
    add [rax], rbx 
    jmp .end_.if_stmt_33
.elseif_stmt_10:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_BOOL]
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_11
    mov rax, qword [rbp - 32]
    mov rdi, rax
    call _varg_getbool_pVA_list
    mov byte [rbp - 10], al
.if_stmt_38:
    mov al, byte [rbp - 10]
    cmp al, 0
    je .else_stmt_9
    mov rax, 4
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_38
.else_stmt_9:
    mov rax, 5
    lea rbx, [rbp - 40]
    add [rbx], rax 
.end_.if_stmt_38:
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    dec qword [rax] 
    jmp .end_.if_stmt_33
.elseif_stmt_11:
    mov rax, qword [rbp - 32]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 48]
    imul rbx, rbx, 8 
    add rax, rbx
    mov rax, qword [rax]
    mov al, byte [rax]
    mov bl, byte [__TYPE_PTR]
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_33
    mov rax, 7
    lea rbx, [rbp - 40]
    add [rbx], rax 
    jmp .end_.if_stmt_33
.end_.if_stmt_33:
.r_for_loop_2:
    lea rax, [rbp - 48]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_2

.end_.for_loop_2:
    mov rax, qword [rbp - 40]
    jmp .ret_from___needed_size_sprintf_pc_pVA_list
.ret_from___needed_size_sprintf_pc_pVA_list:
    add rsp, 56
    pop rbp
    ret
_vsprintf_pc_pVA_list:
    push rbp
    mov rbp, rsp
    sub rsp, 152
    mov qword [rbp - 72], rdi
    mov qword [rbp - 80], rsi
    mov rax, qword [rbp - 72]
    mov rdi, rax
    mov rax, qword [rbp - 80]
    mov rsi, rax
    call ___needed_size_sprintf_pc_pVA_list
    mov qword [rbp - 88], rax
    mov rax, 0
    mov qword [rbp - 96], rax
    mov rax, 0
    mov qword [rbp - 104], rax
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_reset_pVA_list
    mov rax, qword [rbp - 88]
    mov rbx, 1
    add rax, rbx
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 112], rax
    mov rax, qword [rbp - 112]
    mov rdi, rax
    mov al, 0
    mov sil, al
    mov rax, qword [rbp - 88]
    mov rdx, rax
    call _memset_p_c_u64
.while_loop_12:
    mov rax, qword [rbp - 96]
    mov rbx, qword [rbp - 88]
    cmp rax, rbx
    setb al
    cmp al, 0
    je .end_.while_loop_12
.if_stmt_39:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_15
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_40:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_17
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_41:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_14
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_42:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_12
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu8_pVA_list
    mov byte [rbp - 7], al
    mov al, byte [rbp - 7]
    movzx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 8], al
    mov al, byte [rbp - 8]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 9], al
.for_loop_3:
    mov al, byte [rbp - 9]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_3
    mov al, byte [rbp - 7]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 10], dl
    mov al, byte [rbp - 10]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 9]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov al, byte [rbp - 7]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    lea rbx, [rbp - 7]
    mov byte [rbx], al 
.r_for_loop_3:
    lea rax, [rbp - 9]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_3

.end_.for_loop_3:
    mov al, byte [rbp - 8]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_42
.elseif_stmt_12:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_13
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_geti8_pVA_list
    mov byte [rbp - 11], al
.if_stmt_43:
    mov al, byte [rbp - 11]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.if_stmt_43
    mov al, 45
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 96]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov al, byte [rbp - 11]
    neg al
    lea rbx, [rbp - 11]
    mov byte [rbx], al 
    jmp .end_.if_stmt_43
.end_.if_stmt_43:
    mov al, byte [rbp - 11]
    movsx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 12], al
    mov al, byte [rbp - 12]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 13], al
.for_loop_4:
    mov al, byte [rbp - 13]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_4
    mov al, byte [rbp - 11]
    mov bl, 10
    movsx eax, al
    movzx ebx, bl
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 14], dl
    mov al, byte [rbp - 14]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 13]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov al, byte [rbp - 11]
    mov bl, 10
    movsx eax, al
    movzx ebx, bl
    cdq
    idiv ebx
    lea rbx, [rbp - 11]
    mov byte [rbx], al 
.r_for_loop_4:
    lea rax, [rbp - 13]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_4

.end_.for_loop_4:
    mov al, byte [rbp - 12]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_42
.elseif_stmt_13:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_42
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu8_pVA_list
    mov byte [rbp - 15], al
    mov al, byte [rbp - 15]
    movzx rdi, al
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 16], al
    mov al, byte [rbp - 16]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 17], al
.for_loop_5:
    mov al, byte [rbp - 17]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_5
    mov al, 4
    mov bl, byte [rbp - 17]
    imul bl 
    mov bl, byte [rbp - 15]
    movzx bx, bl
    mov cl, al
    shr bx, cl
    mov al, 15
    movzx ax, al
    and bx,ax
    mov byte [rbp - 18], bl
.if_stmt_44:
    mov al, byte [rbp - 18]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_10
    mov al, byte [rbp - 18]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 112]
    mov rax, qword [rax]
    mov cl, byte [rbp - 16]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 17]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_44
.else_stmt_10:
    mov al, byte [rbp - 18]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 16]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 17]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_44:
.r_for_loop_5:
    lea rax, [rbp - 17]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_5

.end_.for_loop_5:
    mov al, byte [rbp - 16]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_42
.end_.if_stmt_42:
    jmp .end_.if_stmt_41
.elseif_stmt_14:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_15
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu16_pVA_list
    mov word [rbp - 48], ax
    mov ax, word [rbp - 48]
    movzx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 19], al
    mov al, byte [rbp - 19]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 20], al
.for_loop_6:
    mov al, byte [rbp - 20]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_6
    mov ax, word [rbp - 48]
    mov bl, 10
    movzx bx, bl
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 21], dl
    mov al, byte [rbp - 21]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 20]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov ax, word [rbp - 48]
    mov bx, 10
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    lea rbx, [rbp - 48]
    mov word [rbx], ax 
.r_for_loop_6:
    lea rax, [rbp - 20]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_6

.end_.for_loop_6:
    mov al, byte [rbp - 19]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_41
.elseif_stmt_15:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_16
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_geti16_pVA_list
    mov word [rbp - 50], ax
.if_stmt_45:
    mov ax, word [rbp - 50]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    setl al
    cmp al, 0
    je .end_.if_stmt_45
    mov al, 45
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 96]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov ax, word [rbp - 50]
    neg ax
    lea rbx, [rbp - 50]
    mov word [rbx], ax 
    jmp .end_.if_stmt_45
.end_.if_stmt_45:
    mov ax, word [rbp - 50]
    movsx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 22], al
    mov al, byte [rbp - 22]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 23], al
.for_loop_7:
    mov al, byte [rbp - 23]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_7
    mov ax, word [rbp - 50]
    mov bl, 10
    movzx bx, bl
    movsx eax, ax
    movzx ebx, bx
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 24], dl
    mov al, byte [rbp - 24]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 23]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov ax, word [rbp - 50]
    mov bx, 10
    movsx eax, ax
    movzx ebx, bx
    cdq
    idiv ebx
    lea rbx, [rbp - 50]
    mov word [rbx], ax 
.r_for_loop_7:
    lea rax, [rbp - 23]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_7

.end_.for_loop_7:
    mov al, byte [rbp - 22]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_41
.elseif_stmt_16:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_41
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu16_pVA_list
    mov word [rbp - 52], ax
    mov ax, word [rbp - 52]
    movzx rdi, ax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 25], al
    mov al, byte [rbp - 25]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 26], al
.for_loop_8:
    mov al, byte [rbp - 26]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_8
    mov al, 4
    mov bl, byte [rbp - 26]
    imul bl 
    mov bx, word [rbp - 52]
    mov cl, al
    shr bx, cl
    mov al, 15
    movzx ax, al
    and bx,ax
    mov byte [rbp - 27], bl
.if_stmt_46:
    mov al, byte [rbp - 27]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_11
    mov al, byte [rbp - 27]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 112]
    mov rax, qword [rax]
    mov cl, byte [rbp - 25]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 26]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_46
.else_stmt_11:
    mov al, byte [rbp - 27]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 25]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 26]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_46:
.r_for_loop_8:
    lea rax, [rbp - 26]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_8

.end_.for_loop_8:
    mov al, byte [rbp - 25]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_41
.end_.if_stmt_41:
    jmp .end_.if_stmt_40
.elseif_stmt_17:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_18
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu32_pVA_list
    mov dword [rbp - 56], eax
    mov eax, dword [rbp - 56]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 28], al
    mov al, byte [rbp - 28]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 29], al
.for_loop_9:
    mov al, byte [rbp - 29]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_9
    mov eax, dword [rbp - 56]
    mov bl, 10
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 30], dl
    mov al, byte [rbp - 30]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 29]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov eax, dword [rbp - 56]
    mov ebx, 10
    xor edx, edx
    div ebx
    lea rbx, [rbp - 56]
    mov dword [rbx], eax 
.r_for_loop_9:
    lea rax, [rbp - 29]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_9

.end_.for_loop_9:
    mov al, byte [rbp - 28]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_40
.elseif_stmt_18:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_19
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_geti32_pVA_list
    mov dword [rbp - 60], eax
.if_stmt_47:
    mov eax, dword [rbp - 60]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_47
    mov al, 45
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 96]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov eax, dword [rbp - 60]
    neg eax
    lea rbx, [rbp - 60]
    mov dword [rbx], eax 
    jmp .end_.if_stmt_47
.end_.if_stmt_47:
    mov eax, dword [rbp - 60]
    movsxd rdi, eax 
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 31], al
    mov al, byte [rbp - 31]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 32], al
.for_loop_10:
    mov al, byte [rbp - 32]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_10
    mov eax, dword [rbp - 60]
    mov bl, 10
    movzx ebx, bl
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 33], dl
    mov al, byte [rbp - 33]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 32]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov eax, dword [rbp - 60]
    mov ebx, 10
    cdq
    idiv ebx
    lea rbx, [rbp - 60]
    mov dword [rbx], eax 
.r_for_loop_10:
    lea rax, [rbp - 32]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_10

.end_.for_loop_10:
    mov al, byte [rbp - 31]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_40
.elseif_stmt_19:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_20
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu32_pVA_list
    mov dword [rbp - 64], eax
    mov eax, dword [rbp - 64]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 34], al
    mov al, byte [rbp - 34]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 35], al
.for_loop_11:
    mov al, byte [rbp - 35]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_11
    mov al, 4
    mov bl, byte [rbp - 35]
    imul bl 
    mov ebx, dword [rbp - 64]
    movsx eax, ax
    mov cl, al
    shr ebx, cl
    mov al, 15
    movzx eax, al
    and ebx,eax
    mov byte [rbp - 36], bl
.if_stmt_48:
    mov al, byte [rbp - 36]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_12
    mov al, byte [rbp - 36]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 112]
    mov rax, qword [rax]
    mov cl, byte [rbp - 34]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 35]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_48
.else_stmt_12:
    mov al, byte [rbp - 36]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 34]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 35]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_48:
.r_for_loop_11:
    lea rax, [rbp - 35]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_11

.end_.for_loop_11:
    mov al, byte [rbp - 34]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_40
.elseif_stmt_20:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_21
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getstr_pVA_list
    mov qword [rbp - 120], rax
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 128], rax
    mov rax, qword [rbp - 112]
    mov rbx, qword [rbp - 96]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 120]
    mov rsi, rax
    mov rax, qword [rbp - 128]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, qword [rbp - 128]
    lea rbx, [rbp - 96]
    add [rbx], rax 
    jmp .end_.if_stmt_40
.elseif_stmt_21:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_22
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getbool_pVA_list
    mov byte [rbp - 37], al
.if_stmt_49:
    mov al, byte [rbp - 37]
    cmp al, 0
    je .else_stmt_13
    mov rax, qword [rbp - 112]
    mov rbx, qword [rbp - 96]
    add rax, rbx
    mov rdi, rax
    lea rax, [Lstr_bstring_0]
    mov rsi, rax
    mov rax, 4
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, 4
    lea rbx, [rbp - 96]
    add [rbx], rax 
    jmp .end_.if_stmt_49
.else_stmt_13:
    mov rax, qword [rbp - 112]
    mov rbx, qword [rbp - 96]
    add rax, rbx
    mov rdi, rax
    lea rax, [Lstr_bstring_1]
    mov rsi, rax
    mov rax, 5
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, 5
    lea rbx, [rbp - 96]
    add [rbx], rax 
.end_.if_stmt_49:
    jmp .end_.if_stmt_40
.elseif_stmt_22:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_40
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_50:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_23
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu64_pVA_list
    mov qword [rbp - 136], rax
    mov rax, qword [rbp - 136]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 38], al
    mov al, byte [rbp - 38]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 39], al
.for_loop_12:
    mov al, byte [rbp - 39]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_12
    mov rax, qword [rbp - 136]
    mov bl, 10
    movzx rbx, bl
    xor rdx, rdx
    div rbx
    mov bl, 48
    movzx rbx, bl
    add rdx, rbx
    mov byte [rbp - 40], dl
    mov al, byte [rbp - 40]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 39]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 136]
    mov rbx, 10
    xor rdx, rdx
    div rbx
    lea rbx, [rbp - 136]
    mov qword [rbx], rax 
.r_for_loop_12:
    lea rax, [rbp - 39]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_12

.end_.for_loop_12:
    mov al, byte [rbp - 38]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_50
.elseif_stmt_23:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_24
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_geti64_pVA_list
    mov qword [rbp - 144], rax
.if_stmt_51:
    mov rax, qword [rbp - 144]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_51
    mov al, 45
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 96]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov rax, qword [rbp - 144]
    neg rax
    lea rbx, [rbp - 144]
    mov qword [rbx], rax 
    jmp .end_.if_stmt_51
.end_.if_stmt_51:
    mov rax, qword [rbp - 144]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 41], al
    mov al, byte [rbp - 41]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 42], al
.for_loop_13:
    mov al, byte [rbp - 42]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_13
    mov rax, qword [rbp - 144]
    mov bl, 10
    movzx rbx, bl
    cqo
    idiv rbx
    mov bl, 48
    movzx rbx, bl
    add rdx, rbx
    mov byte [rbp - 43], dl
    mov al, byte [rbp - 43]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    mov dl, byte [rbp - 42]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 144]
    mov rbx, 10
    cqo
    idiv rbx
    lea rbx, [rbp - 144]
    mov qword [rbx], rax 
.r_for_loop_13:
    lea rax, [rbp - 42]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_13

.end_.for_loop_13:
    mov al, byte [rbp - 41]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_50
.elseif_stmt_24:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_50
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 80]
    mov rdi, rax
    call _varg_getu64_pVA_list
    mov qword [rbp - 152], rax
    mov rax, qword [rbp - 152]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 44], al
    mov al, byte [rbp - 44]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 45], al
.for_loop_14:
    mov al, byte [rbp - 45]
    mov bl, 2
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_14
    mov al, 4
    mov bl, byte [rbp - 45]
    imul bl 
    mov rbx, qword [rbp - 152]
    movsx rax, ax
    mov cl, al
    shr rbx, cl
    mov al, 15
    movzx rax, al
    and rbx,rax
    mov byte [rbp - 46], bl
.if_stmt_52:
    mov al, byte [rbp - 46]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_14
    mov al, byte [rbp - 46]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 112]
    mov rax, qword [rax]
    mov cl, byte [rbp - 44]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 45]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_52
.else_stmt_14:
    mov al, byte [rbp - 46]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 44]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 45]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 96]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_52:
.r_for_loop_14:
    lea rax, [rbp - 45]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_14

.end_.for_loop_14:
    mov al, byte [rbp - 44]
    lea rbx, [rbp - 96]
    movzx rax, al
    add [rbx], rax 
    jmp .end_.if_stmt_50
.end_.if_stmt_50:
    jmp .end_.if_stmt_40
.end_.if_stmt_40:
    jmp .end_.if_stmt_39
.else_stmt_15:
    lea rax, [rbp - 72]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 104]
    add rax, rbx
    mov al, byte [rax]
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 96]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_39:
    jmp .while_loop_12

.end_.while_loop_12:
    mov al, 0
    lea rbx, [rbp - 112]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 96]
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 112]
    jmp .ret_from_vsprintf_pc_pVA_list
.ret_from_vsprintf_pc_pVA_list:
    add rsp, 152
    pop rbp
    ret
_sprintf_pc_vargs:
    push rbp
    mov rbp, rsp
    sub rsp, 232
    mov qword [rbp - 88], rdi
    mov rax, 0
    mov qword [rbp - 96], rax
    mov rax, 0
    mov qword [rbp - 104], rax
    mov rax, 0
    mov qword [rbp - 112], rax
.for_loop_15:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 112]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_15
.if_stmt_53:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 112]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_53
    lea rax, [rbp - 96]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_53
.end_.if_stmt_53:
    lea rax, [rbp - 104]
    mov rbx, qword [rax]
    inc qword [rax] 
.r_for_loop_15:
    lea rax, [rbp - 112]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_15

.end_.for_loop_15:
    mov rax, qword [rbp - 96]
    mov rdi, rax
    call _varg_start_u64
    mov qword [rbp - 120], rax
    mov rax, qword [rbp - 120]
    mov qword [rbp - 128], rax
    mov rax, 0
    mov qword [rbp - 136], rax
    mov rax, 0
    mov qword [rbp - 144], rax
.while_loop_13:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    test al, al
    setne al
    jz .skip_right_1
    mov rbx, qword [rbp - 136]
    mov rcx, qword [rbp - 104]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
.skip_right_1:
    and al, bl
    cmp al, 0
    je .end_.while_loop_13
.if_stmt_54:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_17
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_55:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_29
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_56:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_56
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_57:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_25
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 1], al
    mov al, byte [rbp - 1]
    movzx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_57
.elseif_stmt_25:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_26
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 2], al
.if_stmt_58:
    mov al, byte [rbp - 2]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.if_stmt_58
    lea rax, [rbp - 144]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_58
.end_.if_stmt_58:
    mov al, byte [rbp - 2]
    movsx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_57
.elseif_stmt_26:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_57
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 3], al
    mov al, byte [rbp - 3]
    movzx rdi, al
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_57
.end_.if_stmt_57:
    jmp .end_.if_stmt_56
.end_.if_stmt_56:
.if_stmt_59:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_27
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 46], ax
    mov ax, word [rbp - 46]
    movzx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_59
.elseif_stmt_27:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_28
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 48], ax
.if_stmt_60:
    mov ax, word [rbp - 48]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    setl al
    cmp al, 0
    je .end_.if_stmt_60
    lea rax, [rbp - 144]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_60
.end_.if_stmt_60:
    mov ax, word [rbp - 48]
    movsx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_59
.elseif_stmt_28:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_59
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 50], ax
    mov ax, word [rbp - 50]
    movzx rdi, ax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_59
.end_.if_stmt_59:
    jmp .end_.if_stmt_55
.elseif_stmt_29:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_30
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 60], eax
    mov eax, dword [rbp - 60]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_30:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_31
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 64], eax
.if_stmt_61:
    mov eax, dword [rbp - 64]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_61
    lea rax, [rbp - 144]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_61
.end_.if_stmt_61:
    mov eax, dword [rbp - 64]
    movsxd rdi, eax 
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_31:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_32
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 68], eax
    mov eax, dword [rbp - 68]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_32:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_33
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 152], rax
    mov rax, qword [rbp - 152]
    mov rdi, rax
    call _strlen_pc
    lea rbx, [rbp - 144]
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_33:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 99
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_34
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, 1
    lea rbx, [rbp - 144]
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_34:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_35
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 4], al
.if_stmt_62:
    mov al, byte [rbp - 4]
    cmp al, 0
    je .else_stmt_16
    mov rax, 4
    lea rbx, [rbp - 144]
    add [rbx], rax 
    jmp .end_.if_stmt_62
.else_stmt_16:
    mov rax, 5
    lea rbx, [rbp - 144]
    add [rbx], rax 
.end_.if_stmt_62:
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_55
.elseif_stmt_35:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_55
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_63:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_36
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 160], rax
    mov rax, qword [rbp - 160]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_63
.elseif_stmt_36:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_37
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 168], rax
.if_stmt_64:
    mov rax, qword [rbp - 168]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_64
    lea rax, [rbp - 144]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_64
.end_.if_stmt_64:
    mov rax, qword [rbp - 168]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_63
.elseif_stmt_37:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_63
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 176], rax
    mov rax, qword [rbp - 176]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    lea rbx, [rbp - 144]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_63
.end_.if_stmt_63:
    jmp .end_.if_stmt_55
.end_.if_stmt_55:
    jmp .end_.if_stmt_54
.else_stmt_17:
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    lea rax, [rbp - 144]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_54:
    jmp .while_loop_13

.end_.while_loop_13:
    mov rax, qword [rbp - 144]
    mov rbx, 1
    add rax, rbx
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 184], rax
    mov rax, qword [rbp - 184]
    mov rdi, rax
    mov al, 0
    mov sil, al
    mov rax, qword [rbp - 144]
    mov rdx, rax
    call _memset_p_c_u64
    mov rax, 0
    lea rbx, [rbp - 136]
    mov qword [rbx], rax 
    mov rax, qword [rbp - 128]
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    mov rax, 0
    mov qword [rbp - 192], rax
.while_loop_14:
    mov rax, qword [rbp - 192]
    mov rbx, qword [rbp - 144]
    cmp rax, rbx
    setb al
    cmp al, 0
    je .end_.while_loop_14
.if_stmt_65:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_23
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_66:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_43
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_67:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_40
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_68:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_38
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 5], al
    mov al, byte [rbp - 5]
    movzx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 6], al
    mov al, byte [rbp - 6]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 7], al
.for_loop_16:
    mov al, byte [rbp - 7]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_16
    mov al, byte [rbp - 5]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 8], dl
    mov al, byte [rbp - 8]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 7]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov al, byte [rbp - 5]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    lea rbx, [rbp - 5]
    mov byte [rbx], al 
.r_for_loop_16:
    lea rax, [rbp - 7]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_16

.end_.for_loop_16:
    mov al, byte [rbp - 6]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_68
.elseif_stmt_38:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_39
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 9], al
.if_stmt_69:
    mov al, byte [rbp - 9]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.if_stmt_69
    mov al, 45
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 192]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov al, byte [rbp - 9]
    neg al
    lea rbx, [rbp - 9]
    mov byte [rbx], al 
    jmp .end_.if_stmt_69
.end_.if_stmt_69:
    mov al, byte [rbp - 9]
    movsx rdi, al
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 10], al
    mov al, byte [rbp - 10]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 11], al
.for_loop_17:
    mov al, byte [rbp - 11]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_17
    mov al, byte [rbp - 9]
    mov bl, 10
    movsx eax, al
    movzx ebx, bl
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 12], dl
    mov al, byte [rbp - 12]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 11]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov al, byte [rbp - 9]
    mov bl, 10
    movsx eax, al
    movzx ebx, bl
    cdq
    idiv ebx
    lea rbx, [rbp - 9]
    mov byte [rbx], al 
.r_for_loop_17:
    lea rax, [rbp - 11]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_17

.end_.for_loop_17:
    mov al, byte [rbp - 10]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_68
.elseif_stmt_39:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_68
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 13], al
    mov al, byte [rbp - 13]
    movzx rdi, al
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 14], al
    mov al, byte [rbp - 14]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 15], al
.for_loop_18:
    mov al, byte [rbp - 15]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_18
    mov al, 4
    mov bl, byte [rbp - 15]
    imul bl 
    mov bl, byte [rbp - 13]
    movzx bx, bl
    mov cl, al
    shr bx, cl
    mov al, 15
    movzx ax, al
    and bx,ax
    mov byte [rbp - 16], bl
.if_stmt_70:
    mov al, byte [rbp - 16]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_18
    mov al, byte [rbp - 16]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 184]
    mov rax, qword [rax]
    mov cl, byte [rbp - 14]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 15]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_70
.else_stmt_18:
    mov al, byte [rbp - 16]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 14]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 15]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_70:
.r_for_loop_18:
    lea rax, [rbp - 15]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_18

.end_.for_loop_18:
    mov al, byte [rbp - 14]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_68
.end_.if_stmt_68:
    jmp .end_.if_stmt_67
.elseif_stmt_40:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_41
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 52], ax
    mov ax, word [rbp - 52]
    movzx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 17], al
    mov al, byte [rbp - 17]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 18], al
.for_loop_19:
    mov al, byte [rbp - 18]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_19
    mov ax, word [rbp - 52]
    mov bl, 10
    movzx bx, bl
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 19], dl
    mov al, byte [rbp - 19]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 18]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov ax, word [rbp - 52]
    mov bx, 10
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    lea rbx, [rbp - 52]
    mov word [rbx], ax 
.r_for_loop_19:
    lea rax, [rbp - 18]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_19

.end_.for_loop_19:
    mov al, byte [rbp - 17]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_67
.elseif_stmt_41:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_42
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 54], ax
.if_stmt_71:
    mov ax, word [rbp - 54]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    setl al
    cmp al, 0
    je .end_.if_stmt_71
    mov al, 45
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 192]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov ax, word [rbp - 54]
    neg ax
    lea rbx, [rbp - 54]
    mov word [rbx], ax 
    jmp .end_.if_stmt_71
.end_.if_stmt_71:
    mov ax, word [rbp - 54]
    movsx rdi, ax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 20], al
    mov al, byte [rbp - 20]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 21], al
.for_loop_20:
    mov al, byte [rbp - 21]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_20
    mov ax, word [rbp - 54]
    mov bl, 10
    movzx bx, bl
    movsx eax, ax
    movzx ebx, bx
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 22], dl
    mov al, byte [rbp - 22]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 21]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov ax, word [rbp - 54]
    mov bx, 10
    movsx eax, ax
    movzx ebx, bx
    cdq
    idiv ebx
    lea rbx, [rbp - 54]
    mov word [rbx], ax 
.r_for_loop_20:
    lea rax, [rbp - 21]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_20

.end_.for_loop_20:
    mov al, byte [rbp - 20]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_67
.elseif_stmt_42:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_67
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov ax, word [rax]
    mov word [rbp - 56], ax
    mov ax, word [rbp - 56]
    movzx rdi, ax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 23], al
    mov al, byte [rbp - 23]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 24], al
.for_loop_21:
    mov al, byte [rbp - 24]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_21
    mov al, 4
    mov bl, byte [rbp - 24]
    imul bl 
    mov bx, word [rbp - 56]
    mov cl, al
    shr bx, cl
    mov al, 15
    movzx ax, al
    and bx,ax
    mov byte [rbp - 25], bl
.if_stmt_72:
    mov al, byte [rbp - 25]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_19
    mov al, byte [rbp - 25]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 184]
    mov rax, qword [rax]
    mov cl, byte [rbp - 23]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 24]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_72
.else_stmt_19:
    mov al, byte [rbp - 25]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 23]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 24]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_72:
.r_for_loop_21:
    lea rax, [rbp - 24]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_21

.end_.for_loop_21:
    mov al, byte [rbp - 23]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_67
.end_.if_stmt_67:
    jmp .end_.if_stmt_66
.elseif_stmt_43:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_44
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 72], eax
    mov eax, dword [rbp - 72]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 26], al
    mov al, byte [rbp - 26]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 27], al
.for_loop_22:
    mov al, byte [rbp - 27]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_22
    mov eax, dword [rbp - 72]
    mov bl, 10
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 28], dl
    mov al, byte [rbp - 28]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 27]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov eax, dword [rbp - 72]
    mov ebx, 10
    xor edx, edx
    div ebx
    lea rbx, [rbp - 72]
    mov dword [rbx], eax 
.r_for_loop_22:
    lea rax, [rbp - 27]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_22

.end_.for_loop_22:
    mov al, byte [rbp - 26]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_66
.elseif_stmt_44:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_45
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 76], eax
.if_stmt_73:
    mov eax, dword [rbp - 76]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_73
    mov al, 45
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 192]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov eax, dword [rbp - 76]
    neg eax
    lea rbx, [rbp - 76]
    mov dword [rbx], eax 
    jmp .end_.if_stmt_73
.end_.if_stmt_73:
    mov eax, dword [rbp - 76]
    movsxd rdi, eax 
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 29], al
    mov al, byte [rbp - 29]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 30], al
.for_loop_23:
    mov al, byte [rbp - 30]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_23
    mov eax, dword [rbp - 76]
    mov bl, 10
    movzx ebx, bl
    cdq
    idiv ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    mov byte [rbp - 31], dl
    mov al, byte [rbp - 31]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 30]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov eax, dword [rbp - 76]
    mov ebx, 10
    cdq
    idiv ebx
    lea rbx, [rbp - 76]
    mov dword [rbx], eax 
.r_for_loop_23:
    lea rax, [rbp - 30]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_23

.end_.for_loop_23:
    mov al, byte [rbp - 29]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_66
.elseif_stmt_45:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_46
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov eax, dword [rax]
    mov dword [rbp - 80], eax
    mov eax, dword [rbp - 80]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 32], al
    mov al, byte [rbp - 32]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 33], al
.for_loop_24:
    mov al, byte [rbp - 33]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_24
    mov al, 4
    mov bl, byte [rbp - 33]
    imul bl 
    mov ebx, dword [rbp - 80]
    movsx eax, ax
    mov cl, al
    shr ebx, cl
    mov al, 15
    movzx eax, al
    and ebx,eax
    mov byte [rbp - 34], bl
.if_stmt_74:
    mov al, byte [rbp - 34]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_20
    mov al, byte [rbp - 34]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 184]
    mov rax, qword [rax]
    mov cl, byte [rbp - 32]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 33]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_74
.else_stmt_20:
    mov al, byte [rbp - 34]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 32]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 33]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_74:
.r_for_loop_24:
    lea rax, [rbp - 33]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_24

.end_.for_loop_24:
    mov al, byte [rbp - 32]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_66
.elseif_stmt_46:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_47
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 200], rax
    mov rax, qword [rbp - 200]
    mov rdi, rax
    call _strlen_pc
    mov qword [rbp - 208], rax
    mov rax, qword [rbp - 184]
    mov rbx, qword [rbp - 192]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 200]
    mov rsi, rax
    mov rax, qword [rbp - 208]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, qword [rbp - 208]
    lea rbx, [rbp - 192]
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_66
.elseif_stmt_47:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_48
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov al, byte [rax]
    mov byte [rbp - 35], al
.if_stmt_75:
    mov al, byte [rbp - 35]
    cmp al, 0
    je .else_stmt_21
    mov rax, qword [rbp - 184]
    mov rbx, qword [rbp - 192]
    add rax, rbx
    mov rdi, rax
    lea rax, [Lstr_bstring_2]
    mov rsi, rax
    mov rax, 4
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, 4
    lea rbx, [rbp - 192]
    add [rbx], rax 
    jmp .end_.if_stmt_75
.else_stmt_21:
    mov rax, qword [rbp - 184]
    mov rbx, qword [rbp - 192]
    add rax, rbx
    mov rdi, rax
    lea rax, [Lstr_bstring_3]
    mov rsi, rax
    mov rax, 5
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, 5
    lea rbx, [rbp - 192]
    add [rbx], rax 
.end_.if_stmt_75:
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_66
.elseif_stmt_48:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_66
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_76:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_49
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 216], rax
    mov rax, qword [rbp - 216]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 36], al
    mov al, byte [rbp - 36]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 37], al
.for_loop_25:
    mov al, byte [rbp - 37]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_25
    mov rax, qword [rbp - 216]
    mov bl, 10
    movzx rbx, bl
    xor rdx, rdx
    div rbx
    mov bl, 48
    movzx rbx, bl
    add rdx, rbx
    mov byte [rbp - 38], dl
    mov al, byte [rbp - 38]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 37]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 216]
    mov rbx, 10
    xor rdx, rdx
    div rbx
    lea rbx, [rbp - 216]
    mov qword [rbx], rax 
.r_for_loop_25:
    lea rax, [rbp - 37]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_25

.end_.for_loop_25:
    mov al, byte [rbp - 36]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_76
.elseif_stmt_49:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_50
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 224], rax
.if_stmt_77:
    mov rax, qword [rbp - 224]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_77
    mov al, 45
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 192]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    mov rax, qword [rbp - 224]
    neg rax
    lea rbx, [rbp - 224]
    mov qword [rbx], rax 
    jmp .end_.if_stmt_77
.end_.if_stmt_77:
    mov rax, qword [rbp - 224]
    mov rdi, rax
    mov rax, 10
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 39], al
    mov al, byte [rbp - 39]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 40], al
.for_loop_26:
    mov al, byte [rbp - 40]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_26
    mov rax, qword [rbp - 224]
    mov bl, 10
    movzx rbx, bl
    cqo
    idiv rbx
    mov bl, 48
    movzx rbx, bl
    add rdx, rbx
    mov byte [rbp - 41], dl
    mov al, byte [rbp - 41]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    mov dl, byte [rbp - 40]
    movsx rdx, dl
    add rcx, rdx
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 224]
    mov rbx, 10
    cqo
    idiv rbx
    lea rbx, [rbp - 224]
    mov qword [rbx], rax 
.r_for_loop_26:
    lea rax, [rbp - 40]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_26

.end_.for_loop_26:
    mov al, byte [rbp - 39]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_76
.elseif_stmt_50:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_76
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 120]
    mov rax, qword [rax]
    mov qword [rbp - 232], rax
    mov rax, qword [rbp - 232]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 42], al
    mov al, byte [rbp - 42]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 43], al
.for_loop_27:
    mov al, byte [rbp - 43]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_27
    mov al, 4
    mov bl, byte [rbp - 43]
    imul bl 
    mov rbx, qword [rbp - 232]
    movsx rax, ax
    mov cl, al
    shr rbx, cl
    mov al, 15
    movzx rax, al
    and rbx,rax
    mov byte [rbp - 44], bl
.if_stmt_78:
    mov al, byte [rbp - 44]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_22
    mov al, byte [rbp - 44]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    lea rax, [rbp - 184]
    mov rax, qword [rax]
    mov cl, byte [rbp - 42]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 43]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rax, rdx
    mov byte [rax], bl 
    jmp .end_.if_stmt_78
.else_stmt_22:
    mov al, byte [rbp - 44]
    mov bl, 48
    add al, bl
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov cl, byte [rbp - 42]
    mov rdx, 1
    movzx rcx, cl
    sub rcx , rdx
    mov dl, byte [rbp - 43]
    movsx rdx, dl
    sub rcx , rdx
    mov rdx, qword [rbp - 192]
    add rdx, rcx
    add rbx, rdx
    mov byte [rbx], al 
.end_.if_stmt_78:
.r_for_loop_27:
    lea rax, [rbp - 43]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_27

.end_.for_loop_27:
    mov al, byte [rbp - 42]
    lea rbx, [rbp - 192]
    movzx rax, al
    add [rbx], rax 
    mov rax, qword [rbp - 120]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 120]
    mov [rbx], rax 
    jmp .end_.if_stmt_76
.end_.if_stmt_76:
    jmp .end_.if_stmt_66
.end_.if_stmt_66:
    jmp .end_.if_stmt_65
.else_stmt_23:
    lea rax, [rbp - 88]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 136]
    add rax, rbx
    mov al, byte [rax]
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    lea rcx, [rbp - 192]
    mov rdx, qword [rcx]
    inc qword [rcx] 
    add rbx, rdx
    mov byte [rbx], al 
    lea rax, [rbp - 136]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_65:
    jmp .while_loop_14

.end_.while_loop_14:
    mov al, 0
    lea rbx, [rbp - 184]
    mov rbx, qword [rbx]
    mov rcx, qword [rbp - 192]
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 184]
    jmp .ret_from_sprintf_pc_vargs
.ret_from_sprintf_pc_vargs:
    add rsp, 232
    pop rbp
    ret
