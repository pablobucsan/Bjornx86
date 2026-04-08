section .data
Lstr_bstdio_0: db 116,114,117,101,0
Lstr_bstdio_1: db 102,97,108,115,101,0
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
section .text
_fopen_pc_FileOpenFlags:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov word [rbp - 4], si
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov ax, word [rbp - 4]
    movzx rsi, ax
    mov rax, 438
    mov rdx, rax
    call _open_pc_u64_u64
    mov dword [rbp - 8], eax
.if_stmt_3:
    mov eax, dword [rbp - 8]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_3
    mov rax, qword [null]
    jmp .ret_from_fopen_pc_FileOpenFlags
    jmp .end_.if_stmt_3
.end_.if_stmt_3:
    mov rax, 24
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 24], rax
.if_stmt_4:
    mov rax, qword [rbp - 24]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_4
    mov eax, dword [rbp - 8]
    mov edi, eax
    call _close_i32
    mov rax, qword [null]
    jmp .ret_from_fopen_pc_FileOpenFlags
    jmp .end_.if_stmt_4
.end_.if_stmt_4:
    mov eax, dword [rbp - 8]
    mov rbx, qword [rbp - 24]
    mov dword [rbx + 16], eax 
    mov ax, word [rbp - 4]
    mov rbx, qword [rbp - 24]
    mov word [rbx + 20], ax 
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _strdup_pc
    mov rbx, qword [rbp - 24]
    mov [rbx], rax 
    mov rax, qword [rbp - 24]
    jmp .ret_from_fopen_pc_FileOpenFlags
.ret_from_fopen_pc_FileOpenFlags:
    add rsp, 24
    pop rbp
    ret
_freadBytes_pFile_pc_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov dword [rbp - 4], edx
    mov rax, qword [rbp - 16]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    mov eax, dword [rbp - 4]
    mov edx, eax
    call _read_i32_pc_u32
    mov dword [rbp - 8], eax
    mov eax, dword [rbp - 8]
    jmp .ret_from_freadBytes_pFile_pc_u32
.ret_from_freadBytes_pFile_pc_u32:
    add rsp, 24
    pop rbp
    ret
_freadAll_pFile:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov qword [rbp - 32], rdi
    mov eax, 65536
    mov dword [rbp - 16], eax
    mov al, 1
    mov byte [rbp - 12], al
    mov eax, 0
    mov dword [rbp - 20], eax
    mov eax, dword [rbp - 16]
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 40], rax
.while_loop_1:
    mov al, byte [rbp - 12]
    cmp al, 0
    je .end_.while_loop_1
.if_stmt_5:
    mov eax, dword [rbp - 20]
    mov ebx, dword [rbp - 16]
    add eax, ebx
    mov ebx, dword [rbp - 16]
    mov cl, 2
    movzx ecx, cl
    imul ebx, ecx
    cmp eax, ebx
    setge al
    cmp al, 0
    je .end_.if_stmt_5
    mov eax, dword [rbp - 16]
    mov ebx, 2
    imul eax, ebx
    lea rbx, [rbp - 16]
    mov dword [rbx], eax 
    mov rax, qword [rbp - 40]
    mov rdi, rax
    mov eax, dword [rbp - 16]
    mov rsi, rax
    call _realloc_p_u64
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    jmp .end_.if_stmt_5
.end_.if_stmt_5:
    mov rax, qword [rbp - 32]
    mov rdi, rax
    lea rax, [rbp - 40]
    mov rax, qword [rax]
    mov ebx, dword [rbp - 20]
    add rax, rbx
    mov rsi, rax
    mov eax, dword [rbp - 16]
    mov ebx, dword [rbp - 20]
    sub eax , ebx
    mov edx, eax
    call _freadBytes_pFile_pc_u32
    mov dword [rbp - 24], eax
.if_stmt_6:
    mov eax, dword [rbp - 16]
    mov ebx, dword [rbp - 20]
    sub eax , ebx
    mov ebx, dword [rbp - 24]
    cmp ebx, eax
    setl bl
    cmp bl, 0
    je .end_.if_stmt_6
    mov al, 0
    lea rbx, [rbp - 12]
    mov byte [rbx], al 
    jmp .end_.if_stmt_6
.end_.if_stmt_6:
    mov eax, dword [rbp - 24]
    lea rbx, [rbp - 20]
    add [rbx], rax 
    jmp .while_loop_1

.end_.while_loop_1:
    mov rax, qword [rbp - 40]
    mov rdi, rax
    mov eax, dword [rbp - 20]
    mov rbx, 1
    add rax, rbx
    mov rsi, rax
    call _realloc_p_u64
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    mov al, 0
    lea rbx, [rbp - 40]
    mov rbx, qword [rbx]
    mov ecx, dword [rbp - 20]
    add rbx, rcx
    mov byte [rbx], al 
    mov rax, qword [rbp - 40]
    jmp .ret_from_freadAll_pFile
.ret_from_freadAll_pFile:
    add rsp, 40
    pop rbp
    ret
_fgetc_pFile:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov al, 0
    mov byte [rbp - 12], al
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    lea rax, [rbp - 12]
    mov rsi, rax
    mov eax, 1
    mov edx, eax
    call _read_i32_pc_u32
    mov dword [rbp - 16], eax
.if_stmt_7:
    mov eax, dword [rbp - 16]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    sete al
    cmp al, 0
    je .end_.if_stmt_7
    mov al, byte [EOF]
    jmp .ret_from_fgetc_pFile
    jmp .end_.if_stmt_7
.end_.if_stmt_7:
    mov al, byte [rbp - 12]
    jmp .ret_from_fgetc_pFile
.ret_from_fgetc_pFile:
    add rsp, 24
    pop rbp
    ret
_fclose_pFile:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov eax, dword [rax + 16]
    mov edi, eax
    call _close_i32
.ret_from_fclose_pFile:
    add rsp, 8
    pop rbp
    ret
_makeDirectory_pc_FileMode:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov word [rbp - 12], si
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov ax, word [rbp - 12]
    movzx rsi, ax
    call _mkdir_pc_u64
    mov dword [rbp - 16], eax
    mov eax, dword [rbp - 16]
    jmp .ret_from_makeDirectory_pc_FileMode
.ret_from_makeDirectory_pc_FileMode:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_c:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov byte [rbp - 16], sil
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, byte [rbp - 16]
    mov sil, al
    call _fputc_i32_c
.ret_from_fprint_pFile_c:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_pc:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, 0
    mov qword [rbp - 24], rax
.while_loop_2:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.while_loop_2
    mov rax, qword [rbp - 8]
    mov eax, dword [rax + 16]
    mov edi, eax
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    lea rbx, [rbp - 24]
    mov rcx, qword [rbx]
    inc qword [rbx] 
    add rax, rcx
    mov al, byte [rax]
    mov sil, al
    call _fputc_i32_c
    jmp .while_loop_2

.end_.while_loop_2:
.ret_from_fprint_pFile_pc:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_b:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov byte [rbp - 16], sil
.if_stmt_8:
    mov al, byte [rbp - 16]
    cmp al, 0
    je .else_stmt_0
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [Lstr_bstdio_0]
    mov rsi, rax
    call _fprint_pFile_pc
    jmp .end_.if_stmt_8
.else_stmt_0:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [Lstr_bstdio_1]
    mov rsi, rax
    call _fprint_pFile_pc
.end_.if_stmt_8:
.ret_from_fprint_pFile_b:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov byte [rbp - 11], sil
.if_stmt_9:
    mov al, byte [rbp - 11]
    mov bl, 0
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_9
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 48
    mov sil, al
    call _fputc_i32_c
    jmp .ret_from_fprint_pFile_u8
    jmp .end_.if_stmt_9
.end_.if_stmt_9:
    mov al, 3
    mov byte [rbp - 12], al
    mov al, 0
    lea rbx, [rbp - 16]
    mov cl, byte [rbp - 12]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], al 
.while_loop_3:
    mov al, byte [rbp - 11]
    mov bl, 0
    cmp al, bl
    seta al
    cmp al, 0
    je .end_.while_loop_3
    lea rax, [rbp - 12]
    mov bl, byte [rax]
    dec byte [rax] 
    mov al, byte [rbp - 11]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    lea rbx, [rbp - 16]
    mov cl, byte [rbp - 12]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], dl 
    mov al, byte [rbp - 11]
    mov bl, 10
    movzx eax, al
    movzx ebx, bl
    xor edx, edx
    div ebx
    lea rbx, [rbp - 11]
    mov byte [rbx], al 
    jmp .while_loop_3

.end_.while_loop_3:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [rbp - 16]
    mov bl, byte [rbp - 12]
    movsx rbx, bl
    add rax, rbx
    mov rsi, rax
    call _fprint_pFile_pc
.ret_from_fprint_pFile_u8:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_i8:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov byte [rbp - 16], sil
.if_stmt_10:
    mov al, byte [rbp - 16]
    mov bl, 0
    cmp al, bl
    setl al
    cmp al, 0
    je .end_.if_stmt_10
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 45
    mov sil, al
    call _fputc_i32_c
    mov al, byte [rbp - 16]
    neg al
    lea rbx, [rbp - 16]
    mov byte [rbx], al 
    jmp .end_.if_stmt_10
.end_.if_stmt_10:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov al, byte [rbp - 16]
    mov sil, al
    call _fprint_pFile_u8
.ret_from_fprint_pFile_i8:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_u16:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov word [rbp - 10], si
.if_stmt_11:
    mov ax, word [rbp - 10]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    sete al
    cmp al, 0
    je .end_.if_stmt_11
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 48
    mov sil, al
    call _fputc_i32_c
    jmp .ret_from_fprint_pFile_u16
    jmp .end_.if_stmt_11
.end_.if_stmt_11:
    mov al, 5
    mov byte [rbp - 8], al
    mov al, 0
    lea rbx, [rbp - 16]
    mov cl, byte [rbp - 8]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], al 
.while_loop_4:
    mov ax, word [rbp - 10]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    seta al
    cmp al, 0
    je .end_.while_loop_4
    lea rax, [rbp - 8]
    mov bl, byte [rax]
    dec byte [rax] 
    mov ax, word [rbp - 10]
    mov bl, 10
    movzx bx, bl
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    lea rbx, [rbp - 16]
    mov cl, byte [rbp - 8]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], dl 
    mov ax, word [rbp - 10]
    mov bx, 10
    movzx eax, ax
    movzx ebx, bx
    xor edx, edx
    div ebx
    lea rbx, [rbp - 10]
    mov word [rbx], ax 
    jmp .while_loop_4

.end_.while_loop_4:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [rbp - 16]
    mov bl, byte [rbp - 8]
    movsx rbx, bl
    add rax, rbx
    mov rsi, rax
    call _fprint_pFile_pc
.ret_from_fprint_pFile_u16:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_i16:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov word [rbp - 16], si
.if_stmt_12:
    mov ax, word [rbp - 16]
    mov bl, 0
    movzx bx, bl
    cmp ax, bx
    setl al
    cmp al, 0
    je .end_.if_stmt_12
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 45
    mov sil, al
    call _fputc_i32_c
    mov ax, word [rbp - 16]
    neg ax
    lea rbx, [rbp - 16]
    mov word [rbx], ax 
    jmp .end_.if_stmt_12
.end_.if_stmt_12:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov ax, word [rbp - 16]
    mov si, ax
    call _fprint_pFile_u16
.ret_from_fprint_pFile_i16:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov qword [rbp - 24], rdi
    mov dword [rbp - 16], esi
.if_stmt_13:
    mov eax, dword [rbp - 16]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    sete al
    cmp al, 0
    je .end_.if_stmt_13
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 48
    mov sil, al
    call _fputc_i32_c
    jmp .ret_from_fprint_pFile_u32
    jmp .end_.if_stmt_13
.end_.if_stmt_13:
    mov al, 10
    mov byte [rbp - 12], al
    mov al, 0
    lea rbx, [rbp - 40]
    mov cl, byte [rbp - 12]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], al 
.while_loop_5:
    mov eax, dword [rbp - 16]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    seta al
    cmp al, 0
    je .end_.while_loop_5
    lea rax, [rbp - 12]
    mov bl, byte [rax]
    dec byte [rax] 
    mov eax, dword [rbp - 16]
    mov bl, 10
    movzx ebx, bl
    xor edx, edx
    div ebx
    mov bl, 48
    movzx ebx, bl
    add edx, ebx
    lea rbx, [rbp - 40]
    mov cl, byte [rbp - 12]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], dl 
    mov eax, dword [rbp - 16]
    mov ebx, 10
    xor edx, edx
    div ebx
    lea rbx, [rbp - 16]
    mov dword [rbx], eax 
    jmp .while_loop_5

.end_.while_loop_5:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [rbp - 40]
    mov bl, byte [rbp - 12]
    movsx rbx, bl
    add rax, rbx
    mov rsi, rax
    call _fprint_pFile_pc
.ret_from_fprint_pFile_u32:
    add rsp, 40
    pop rbp
    ret
_fprint_pFile_i32:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 24], rdi
    mov dword [rbp - 16], esi
.if_stmt_14:
    mov eax, dword [rbp - 16]
    mov bl, 0
    movzx ebx, bl
    cmp eax, ebx
    setl al
    cmp al, 0
    je .end_.if_stmt_14
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 45
    mov sil, al
    call _fputc_i32_c
    mov eax, dword [rbp - 16]
    neg eax
    lea rbx, [rbp - 16]
    mov dword [rbx], eax 
    jmp .end_.if_stmt_14
.end_.if_stmt_14:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov eax, dword [rbp - 16]
    mov esi, eax
    call _fprint_pFile_u32
.ret_from_fprint_pFile_i32:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 56
    mov qword [rbp - 24], rdi
    mov qword [rbp - 32], rsi
.if_stmt_15:
    mov rax, qword [rbp - 32]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_15
    mov rax, qword [rbp - 24]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 48
    mov sil, al
    call _fputc_i32_c
    jmp .ret_from_fprint_pFile_u64
    jmp .end_.if_stmt_15
.end_.if_stmt_15:
    mov al, 20
    mov byte [rbp - 16], al
    mov al, 0
    lea rbx, [rbp - 56]
    mov cl, byte [rbp - 16]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], al 
.while_loop_6:
    mov rax, qword [rbp - 32]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    seta al
    cmp al, 0
    je .end_.while_loop_6
    lea rax, [rbp - 16]
    mov bl, byte [rax]
    dec byte [rax] 
    mov rax, qword [rbp - 32]
    mov bl, 10
    movzx rbx, bl
    xor rdx, rdx
    div rbx
    mov bl, 48
    movzx rbx, bl
    add rdx, rbx
    lea rbx, [rbp - 56]
    mov cl, byte [rbp - 16]
    movsx rcx, cl
    add rbx, rcx
    mov byte [rbx], dl 
    mov rax, qword [rbp - 32]
    mov rbx, 10
    xor rdx, rdx
    div rbx
    lea rbx, [rbp - 32]
    mov qword [rbx], rax 
    jmp .while_loop_6

.end_.while_loop_6:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    lea rax, [rbp - 56]
    mov bl, byte [rbp - 16]
    movsx rbx, bl
    add rax, rbx
    mov rsi, rax
    call _fprint_pFile_pc
.ret_from_fprint_pFile_u64:
    add rsp, 56
    pop rbp
    ret
_fprint_pFile_i64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
.if_stmt_16:
    mov rax, qword [rbp - 24]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_16
    mov rax, qword [rbp - 16]
    mov eax, dword [rax + 16]
    mov edi, eax
    mov al, 45
    mov sil, al
    call _fputc_i32_c
    mov rax, qword [rbp - 24]
    neg rax
    lea rbx, [rbp - 24]
    mov qword [rbx], rax 
    jmp .end_.if_stmt_16
.end_.if_stmt_16:
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    call _fprint_pFile_u64
.ret_from_fprint_pFile_i64:
    add rsp, 24
    pop rbp
    ret
_fprintHex_pFile_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov al, 15
    mov byte [rbp - 4], al
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov rax, 16
    mov rsi, rax
    call _digitsof_i64_u64
    mov byte [rbp - 5], al
    mov al, byte [rbp - 5]
    mov bl, 1
    sub al , bl
    mov byte [rbp - 6], al
.for_loop_0:
    mov al, byte [rbp - 6]
    mov bl, 0
    cmp al, bl
    setge al
    cmp al, 0
    je .end_.for_loop_0
    mov al, 4
    mov bl, byte [rbp - 6]
    imul bl 
    mov rbx, qword [rbp - 24]
    movsx rax, ax
    mov cl, al
    shr rbx, cl
    mov al, byte [rbp - 4]
    movzx rax, al
    and rbx,rax
    mov byte [rbp - 7], bl
.if_stmt_17:
    mov al, byte [rbp - 7]
    mov bl, 9
    cmp al, bl
    seta al
    cmp al, 0
    je .else_stmt_1
    mov al, byte [rbp - 7]
    mov bl, 10
    sub al , bl
    mov bl, 65
    add bl, al
    mov byte [rbp - 8], bl
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov al, byte [rbp - 8]
    mov sil, al
    call _fprint_pFile_c
    jmp .end_.if_stmt_17
.else_stmt_1:
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov al, byte [rbp - 7]
    mov sil, al
    call _fprint_pFile_u8
.end_.if_stmt_17:
.r_for_loop_0:
    lea rax, [rbp - 6]
    mov bl, byte [rax]
    dec byte [rax] 
    jmp .for_loop_0

.end_.for_loop_0:
.ret_from_fprintHex_pFile_u64:
    add rsp, 24
    pop rbp
    ret
_fprint_pFile_p:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, qword [rbp - 16]
    mov qword [rbp - 24], rax
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    call _fprintHex_pFile_u64
.ret_from_fprint_pFile_p:
    add rsp, 24
    pop rbp
    ret
_fprintf_pFile_pc_vargs:
    push rbp
    mov rbp, rsp
    sub rsp, 72
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, 0
    mov qword [rbp - 24], rax
    mov rax, 0
    mov qword [rbp - 32], rax
    mov rax, 0
    mov qword [rbp - 40], rax
.for_loop_1:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_1
.if_stmt_18:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_18
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_18
.end_.if_stmt_18:
    lea rax, [rbp - 32]
    mov rbx, qword [rax]
    inc qword [rax] 
.r_for_loop_1:
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_1

.end_.for_loop_1:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _varg_start_u64
    mov qword [rbp - 48], rax
    mov rax, 0
    mov qword [rbp - 56], rax
.while_loop_7:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    test al, al
    setne al
    jz .skip_right_0
    mov rbx, qword [rbp - 56]
    mov rcx, qword [rbp - 32]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
.skip_right_0:
    and al, bl
    cmp al, 0
    je .end_.while_loop_7
.if_stmt_19:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_2
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_20:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_5
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_21:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_2
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_22:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_0
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_u8
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_22
.elseif_stmt_0:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_1
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_i8
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_22
.elseif_stmt_1:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_22
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    movzx rsi, al
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_22
.end_.if_stmt_22:
    jmp .end_.if_stmt_21
.elseif_stmt_2:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_3
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    mov si, ax
    call _fprint_pFile_u16
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_21
.elseif_stmt_3:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_4
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    mov si, ax
    call _fprint_pFile_i16
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_21
.elseif_stmt_4:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_21
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    movzx rsi, ax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_21
.end_.if_stmt_21:
    jmp .end_.if_stmt_20
.elseif_stmt_5:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_6
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov esi, eax
    call _fprint_pFile_u32
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_6:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_7
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov esi, eax
    call _fprint_pFile_i32
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_7:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_8
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov rsi, rax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_8:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_9
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_23:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_23
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprint_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_23
.end_.if_stmt_23:
.if_stmt_24:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_24
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprint_pFile_i64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_24
.end_.if_stmt_24:
.if_stmt_25:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_25
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_25
.end_.if_stmt_25:
    jmp .end_.if_stmt_20
.elseif_stmt_9:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_10
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov qword [rbp - 64], rax
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 64]
    mov rsi, rax
    call _fprint_pFile_pc
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_10:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 99
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_11
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_c
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_11:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_12
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_b
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_12:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 112
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_13
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov qword [rbp - 72], rax
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [rbp - 72]
    mov rsi, rax
    call _fprint_pFile_p
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_20
.elseif_stmt_13:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_20
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_20
.end_.if_stmt_20:
    jmp .end_.if_stmt_19
.else_stmt_2:
    mov rax, qword [rbp - 8]
    mov eax, dword [rax + 16]
    mov edi, eax
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov sil, al
    call _fputc_i32_c
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_19:
    jmp .while_loop_7

.end_.while_loop_7:
.ret_from_fprintf_pFile_pc_vargs:
    add rsp, 72
    pop rbp
    ret
_printf_pc_vargs:
    push rbp
    mov rbp, rsp
    sub rsp, 72
    mov qword [rbp - 16], rdi
    mov rax, 0
    mov qword [rbp - 24], rax
    mov rax, 0
    mov qword [rbp - 32], rax
    mov rax, 0
    mov qword [rbp - 40], rax
.for_loop_2:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    cmp al, 0
    je .end_.for_loop_2
.if_stmt_26:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_26
    lea rax, [rbp - 24]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .end_.if_stmt_26
.end_.if_stmt_26:
    lea rax, [rbp - 32]
    mov rbx, qword [rax]
    inc qword [rax] 
.r_for_loop_2:
    lea rax, [rbp - 40]
    mov rbx, qword [rax]
    inc qword [rax] 
    jmp .for_loop_2

.end_.for_loop_2:
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _varg_start_u64
    mov qword [rbp - 48], rax
    mov rax, 0
    mov qword [rbp - 56], rax
.while_loop_8:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 0
    cmp al, bl
    setne al
    test al, al
    setne al
    jz .skip_right_1
    mov rbx, qword [rbp - 56]
    mov rcx, qword [rbp - 32]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
.skip_right_1:
    and al, bl
    cmp al, 0
    je .end_.while_loop_8
.if_stmt_27:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 37
    cmp al, bl
    sete al
    cmp al, 0
    je .else_stmt_3
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_28:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_19
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_29:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 104
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_16
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_30:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_14
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_u8
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_30
.elseif_stmt_14:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_15
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_i8
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_30
.elseif_stmt_15:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_30
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    movzx rsi, al
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_30
.end_.if_stmt_30:
    jmp .end_.if_stmt_29
.elseif_stmt_16:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_17
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    mov si, ax
    call _fprint_pFile_u16
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_29
.elseif_stmt_17:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_18
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    mov si, ax
    call _fprint_pFile_i16
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_29
.elseif_stmt_18:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_29
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov ax, word [rax]
    movzx rsi, ax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_29
.end_.if_stmt_29:
    jmp .end_.if_stmt_28
.elseif_stmt_19:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_20
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov esi, eax
    call _fprint_pFile_u32
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_20:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_21
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov esi, eax
    call _fprint_pFile_i32
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_21:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_22
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov eax, dword [rax]
    mov rsi, rax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_22:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 108
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_25
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.if_stmt_31:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 117
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_23
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprint_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_31
.elseif_stmt_23:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 105
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_24
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprint_pFile_i64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_31
.elseif_stmt_24:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 120
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_31
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov rsi, rax
    call _fprintHex_pFile_u64
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_31
.end_.if_stmt_31:
    jmp .end_.if_stmt_28
.elseif_stmt_25:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 115
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_26
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov qword [rbp - 64], rax
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 64]
    mov rsi, rax
    call _fprint_pFile_pc
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_26:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 99
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_27
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_c
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_27:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 66
    cmp al, bl
    sete al
    cmp al, 0
    je .elseif_stmt_28
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 48]
    mov al, byte [rax]
    mov sil, al
    call _fprint_pFile_b
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.elseif_stmt_28:
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov bl, 112
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_28
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    mov qword [rbp - 72], rax
    mov rax, qword [STDOUT]
    mov rdi, rax
    mov rax, qword [rbp - 72]
    mov rsi, rax
    call _fprint_pFile_p
    mov rax, qword [rbp - 48]
    mov rdi, rax
    call _varg_next_p
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .end_.if_stmt_28
.end_.if_stmt_28:
    jmp .end_.if_stmt_27
.else_stmt_3:
    mov eax, 1
    mov edi, eax
    lea rax, [rbp - 16]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    add rax, rbx
    mov al, byte [rax]
    mov sil, al
    call _fputc_i32_c
    lea rax, [rbp - 56]
    mov rbx, qword [rax]
    inc qword [rax] 
.end_.if_stmt_27:
    jmp .while_loop_8

.end_.while_loop_8:
.ret_from_printf_pc_vargs:
    add rsp, 72
    pop rbp
    ret
