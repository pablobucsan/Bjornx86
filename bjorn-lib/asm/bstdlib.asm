section .data
Lstr_bstdlib_0: db 66,115,116,100,108,105,98,32,45,32,102,114,101,101,58,32,68,111,117,98,108,101,32,102,114,101,101,32,100,101,116,101,99,116,101,100,10,0
Lstr_bstdlib_1: db 66,115,116,100,108,105,98,32,45,32,114,101,97,108,108,111,99,32,101,120,116,101,110,100,32,105,110,32,112,108,97,99,101,58,32,67,111,97,108,101,115,99,101,110,116,32,110,111,116,32,102,111,117,110,100,32,105,110,32,102,114,101,101,32,108,105,115,116,10,0
Lstr_bstdlib_2: db 66,115,116,100,108,105,98,32,45,32,114,101,97,108,108,111,99,58,32,82,101,97,108,108,111,99,97,116,105,110,103,32,97,32,102,114,101,101,100,32,112,111,105,110,116,101,114,46,10,0
Lstr_bstdlib_3: db 66,115,116,100,108,105,98,32,45,32,114,101,97,108,108,111,99,58,32,80,111,105,110,116,101,114,32,100,111,101,115,32,110,111,116,32,98,101,108,111,110,103,32,116,111,32,97,110,121,32,104,101,97,112,46,10,0
Lstr_bstdlib_4: db 80,111,105,110,116,101,114,58,32,48,120,37,112,10,0
Lstr_bstdlib_5: db 76,97,115,116,32,72,101,97,112,32,115,116,97,114,116,58,32,48,120,37,112,44,32,101,110,100,58,32,48,120,37,112,10,0
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
PROT_NONE: dq 0
PROT_READ: dq 1
PROT_WRITE: dq 2
PROT_EXEC: dq 4
MAP_SHARED: dq 1
MAP_PRIVATE: dq 2
MAP_ANONYMOUS: dq 32
PAGE_SIZE: dq 4096
__HEADER_SIZE: dq 32
__HEAP_HEADER_SIZE: dq 32
__REQSIZE_OFFSET: dq 8
__TOTSIZE_OFFSET: dq 16
__MAGIC_OFFSET: dq 24
__MAGIC_ALLOCATED: dq 43235720943
__MAGIC_FREED: dq 65480277743
__MAX_ALIGN_SIZE: dq 32
__heap_init: db 0
__heap: dq 0
section .text
___align_to_u64_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 8]
    add rax, rbx
    mov rbx, 1
    sub rax , rbx
    mov rbx, qword [rbp - 16]
    xor rdx, rdx
    div rbx
    mov rbx, qword [rbp - 16]
    mov rcx, 1
    sub rbx , rcx
    mov rcx, qword [rbp - 8]
    add rcx, rbx
    sub rcx , rdx
    mov qword [rbp - 24], rcx
    mov rax, qword [rbp - 24]
    jmp .ret_from___align_to_u64_u64
.ret_from___align_to_u64_u64:
    add rsp, 24
    pop rbp
    ret
___createFreeBlock_p_u64_p__FreeBlock:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov qword [rbp - 32], rdx
    mov rax, qword [rbp - 16]
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 32]
    mov rbx, qword [rbp - 40]
    mov [rbx], rax 
    mov rax, 0
    mov rbx, qword [rbp - 40]
    mov qword [rbx + 24], rax 
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    mov qword [rbx + 16], rax 
    mov rax, qword [rbp - 40]
    jmp .ret_from___createFreeBlock_p_u64_p__FreeBlock
.ret_from___createFreeBlock_p_u64_p__FreeBlock:
    add rsp, 40
    pop rbp
    ret
___mergeFreeBlocks_p__FreeBlock_p__FreeBlock:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov rax, qword [rbp - 24]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 16]
    mov [rbx], rax 
    mov rax, qword [rbp - 24]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 16]
    add [rbx + 16], rax 
.ret_from___mergeFreeBlocks_p__FreeBlock_p__FreeBlock:
    add rsp, 24
    pop rbp
    ret
___tryMergeFreeBlocks_p__FreeBlock_p__FreeBlock:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
.if_stmt_29:
    mov rax, qword [rbp - 24]
    mov rax, qword [rax]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_0
    mov rbx, qword [rbp - 24]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 24]
    add rcx, rbx
    mov rbx, qword [rbp - 24]
    mov rbx, qword [rbx]
    cmp rcx, rbx
    sete cl
    test cl, cl
    setne cl
.skip_right_0:
    and al, cl
    cmp al, 0
    je .end_.if_stmt_29
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rax, qword [rax]
    mov rsi, rax
    call ___mergeFreeBlocks_p__FreeBlock_p__FreeBlock
    jmp .end_.if_stmt_29
.end_.if_stmt_29:
.if_stmt_30:
    mov rax, qword [rbp - 16]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_1
    mov rbx, qword [rbp - 16]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 16]
    add rcx, rbx
    mov rbx, qword [rbp - 24]
    cmp rcx, rbx
    sete cl
    test cl, cl
    setne cl
.skip_right_1:
    and al, cl
    cmp al, 0
    je .end_.if_stmt_30
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    call ___mergeFreeBlocks_p__FreeBlock_p__FreeBlock
    jmp .end_.if_stmt_30
.end_.if_stmt_30:
.ret_from___tryMergeFreeBlocks_p__FreeBlock_p__FreeBlock:
    add rsp, 24
    pop rbp
    ret
___createNewHeap_u64_p__Heap:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov rax, qword [rbp - 16]
    mov rbx, qword [__HEAP_HEADER_SIZE]
    add rax, rbx
    mov qword [rbp - 32], rax
    mov rax, qword [null]
    mov rdi, rax
    mov rax, qword [rbp - 32]
    mov rsi, rax
    mov rax, qword [PROT_READ]
    mov rbx, qword [PROT_WRITE]
    or rax,rbx
    mov rdx, rax
    mov rax, qword [MAP_PRIVATE]
    mov rbx, qword [MAP_ANONYMOUS]
    or rax,rbx
    mov rcx, rax
    mov rax, -1
    mov r8, rax
    mov rax, 0
    mov r9, rax
    call _mmap_p_u64_u64_u64_u64_u64
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 32]
    mov rdi, rax
    mov rax, qword [PAGE_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov rbx, qword [rbp - 40]
    mov qword [rbx + 24], rax 
    mov rax, qword [rbp - 40]
    mov rax, qword [rax + 24]
    mov rbx, 32
    sub rax , rbx
    mov rbx, qword [rbp - 40]
    mov qword [rbx + 16], rax 
    mov rax, qword [rbp - 40]
    mov rbx, 32
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 40]
    mov rax, qword [rax + 16]
    mov rsi, rax
    mov rax, qword [null]
    mov rdx, rax
    call ___createFreeBlock_p_u64_p__FreeBlock
    mov rbx, qword [rbp - 40]
    mov [rbx + 8], rax 
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    mov [rbx], rax 
    mov rax, qword [rbp - 40]
    jmp .ret_from___createNewHeap_u64_p__Heap
.ret_from___createNewHeap_u64_p__Heap:
    add rsp, 40
    pop rbp
    ret
___initHeap_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    mov rdi, rax
    mov rax, qword [null]
    mov rsi, rax
    call ___createNewHeap_u64_p__Heap
    lea rbx, qword [__heap]
    mov [rbx], rax 
    mov al, 1
    lea rbx, qword [__heap_init]
    mov byte [rbx], al 
.ret_from___initHeap_u64:
    add rsp, 8
    pop rbp
    ret
___isBlockInFreeList_p__Heap_p__FreeBlock:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, qword [rbp - 8]
    mov rax, qword [rax + 8]
    mov qword [rbp - 24], rax
.while_loop_8:
    mov rax, qword [rbp - 24]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    cmp al, 0
    je .end_.while_loop_8
.if_stmt_31:
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 16]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_31
    mov al, 1
    jmp .ret_from___isBlockInFreeList_p__Heap_p__FreeBlock
    jmp .end_.if_stmt_31
.end_.if_stmt_31:
    mov rax, qword [rbp - 24]
    mov rax, qword [rax]
    lea rbx, [rbp - 24]
    mov [rbx], rax 
    jmp .while_loop_8

.end_.while_loop_8:
    mov al, 0
    jmp .ret_from___isBlockInFreeList_p__Heap_p__FreeBlock
.ret_from___isBlockInFreeList_p__Heap_p__FreeBlock:
    add rsp, 24
    pop rbp
    ret
_malloc_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 104
    mov qword [rbp - 16], rdi
    mov rax, qword [rbp - 16]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [__MAX_ALIGN_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov qword [rbp - 24], rax
.if_stmt_32:
    mov al, byte [__heap_init]
    mov bl, 0
    cmp al, bl
    sete al
    cmp al, 0
    je .end_.if_stmt_32
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call ___initHeap_u64
    jmp .end_.if_stmt_32
.end_.if_stmt_32:
    mov rax, qword [null]
    mov qword [rbp - 32], rax
    mov rax, qword [__heap]
    mov qword [rbp - 40], rax
.while_loop_9:
    mov rax, qword [rbp - 40]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_2
    mov rbx, qword [rbp - 40]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 24]
    cmp rcx, rbx
    setae cl
    test cl, cl
    setne cl
.skip_right_2:
    and al, cl
    cmp al, 0
    je .end_.while_loop_9
    mov rax, qword [rbp - 40]
    lea rbx, [rbp - 32]
    mov [rbx], rax 
    mov rax, qword [rbp - 40]
    mov rax, qword [rax]
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    jmp .while_loop_9

.end_.while_loop_9:
.if_stmt_33:
    mov rax, qword [rbp - 40]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_33
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov rax, qword [null]
    mov rsi, rax
    call ___createNewHeap_u64_p__Heap
    mov qword [rbp - 48], rax
.if_stmt_34:
    mov rax, qword [rbp - 48]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_34
    mov rax, qword [null]
    jmp .ret_from_malloc_u64
    jmp .end_.if_stmt_34
.end_.if_stmt_34:
    mov rax, qword [rbp - 48]
    mov rbx, qword [rbp - 32]
    mov [rbx], rax 
    mov rax, qword [rbp - 48]
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    jmp .end_.if_stmt_33
.end_.if_stmt_33:
    mov rax, qword [null]
    mov qword [rbp - 56], rax
    mov rax, qword [rbp - 40]
    mov rax, qword [rax + 8]
    mov qword [rbp - 64], rax
.while_loop_10:
    mov rax, qword [rbp - 64]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    cmp al, 0
    je .end_.while_loop_10
.if_stmt_35:
    mov rax, qword [rbp - 64]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 24]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .elseif_stmt_29
    mov rax, qword [__MAGIC_ALLOCATED]
    mov rbx, qword [rbp - 64]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 64]
    mov qword [rbx + 24], rax 
.if_stmt_36:
    mov rax, qword [rbp - 56]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_4
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_36
.else_stmt_4:
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    mov [rbx], rax 
.end_.if_stmt_36:
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    sub [rbx + 16], rax 
    mov rax, qword [rbp - 64]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    jmp .ret_from_malloc_u64
    jmp .end_.if_stmt_35
.elseif_stmt_29:
    mov rax, qword [rbp - 64]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 24]
    cmp rax, rbx
    seta al
    cmp al, 0
    je .end_.if_stmt_35
    mov rax, qword [rbp - 64]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov qword [rbp - 72], rax
    mov rax, qword [rbp - 64]
    mov rax, qword [rax + 16]
    mov qword [rbp - 80], rax
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov qword [rbp - 88], rax
    mov rax, qword [__MAGIC_ALLOCATED]
    mov rbx, qword [rbp - 64]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 64]
    mov qword [rbx + 24], rax 
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 64]
    mov qword [rbx + 16], rax 
.if_stmt_37:
    mov rax, qword [rbp - 80]
    mov rbx, qword [rbp - 24]
    sub rax , rbx
    mov rbx, qword [__HEADER_SIZE]
    mov rcx, qword [__MAX_ALIGN_SIZE]
    add rbx, rcx
    cmp rax, rbx
    setge al
    cmp al, 0
    je .else_stmt_6
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 24]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 80]
    mov rbx, qword [rbp - 24]
    sub rax , rbx
    mov rsi, rax
    mov rax, qword [rbp - 88]
    mov rdx, rax
    call ___createFreeBlock_p_u64_p__FreeBlock
    lea rbx, [rbp - 64]
    mov [rbx], rax 
.if_stmt_38:
    mov rax, qword [rbp - 56]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_5
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 40]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_38
.else_stmt_5:
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 56]
    mov [rbx], rax 
.end_.if_stmt_38:
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    sub [rbx + 16], rax 
    jmp .end_.if_stmt_37
.else_stmt_6:
.if_stmt_39:
    mov rax, qword [rbp - 56]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_7
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 40]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_39
.else_stmt_7:
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 56]
    mov [rbx], rax 
.end_.if_stmt_39:
    mov rax, qword [rbp - 80]
    mov rbx, qword [rbp - 40]
    sub [rbx + 16], rax 
.end_.if_stmt_37:
    mov rax, qword [rbp - 72]
    jmp .ret_from_malloc_u64
    jmp .end_.if_stmt_35
.end_.if_stmt_35:
    mov rax, qword [rbp - 64]
    lea rbx, [rbp - 56]
    mov [rbx], rax 
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    lea rbx, [rbp - 64]
    mov [rbx], rax 
.if_stmt_40:
    mov rax, qword [rbp - 64]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_40
    mov rax, qword [rbp - 40]
    lea rbx, [rbp - 32]
    mov [rbx], rax 
    mov rax, qword [rbp - 40]
    mov rax, qword [rax]
    mov qword [rbp - 96], rax
.while_loop_11:
    mov rax, qword [rbp - 96]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_3
    mov rbx, qword [rbp - 96]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 24]
    cmp rcx, rbx
    setae cl
    test cl, cl
    setne cl
.skip_right_3:
    and al, cl
    cmp al, 0
    je .end_.while_loop_11
    mov rax, qword [rbp - 96]
    lea rbx, [rbp - 32]
    mov [rbx], rax 
    mov rax, qword [rbp - 96]
    mov rax, qword [rax]
    lea rbx, [rbp - 96]
    mov [rbx], rax 
    jmp .while_loop_11

.end_.while_loop_11:
.if_stmt_41:
    mov rax, qword [rbp - 96]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_41
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov rax, qword [null]
    mov rsi, rax
    call ___createNewHeap_u64_p__Heap
    mov qword [rbp - 104], rax
.if_stmt_42:
    mov rax, qword [rbp - 104]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_42
    mov rax, qword [null]
    jmp .ret_from_malloc_u64
    jmp .end_.if_stmt_42
.end_.if_stmt_42:
    mov rax, qword [rbp - 104]
    mov rbx, qword [rbp - 32]
    mov [rbx], rax 
    mov rax, qword [rbp - 104]
    lea rbx, [rbp - 96]
    mov [rbx], rax 
    jmp .end_.if_stmt_41
.end_.if_stmt_41:
    mov rax, qword [rbp - 96]
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    mov rax, qword [null]
    lea rbx, [rbp - 56]
    mov [rbx], rax 
    mov rax, qword [rbp - 40]
    mov rax, qword [rax + 8]
    lea rbx, [rbp - 64]
    mov [rbx], rax 
    jmp .end_.if_stmt_40
.end_.if_stmt_40:
    jmp .while_loop_10

.end_.while_loop_10:
    mov rax, qword [null]
    jmp .ret_from_malloc_u64
.ret_from_malloc_u64:
    add rsp, 104
    pop rbp
    ret
_free_p:
    push rbp
    mov rbp, rsp
    sub rsp, 72
    mov qword [rbp - 8], rdi
.if_stmt_43:
    mov rax, qword [rbp - 8]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_43
    jmp .ret_from_free_p
    jmp .end_.if_stmt_43
.end_.if_stmt_43:
    mov rax, qword [rbp - 8]
    mov rbx, qword [__REQSIZE_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 16], rax
    mov rax, qword [rbp - 8]
    mov rbx, qword [__TOTSIZE_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 24], rax
    mov rax, qword [rbp - 8]
    mov rbx, qword [__MAGIC_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 32], rax
.if_stmt_44:
    mov rax, qword [rbp - 32]
    mov rbx, qword [__MAGIC_FREED]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_44
    lea rax, [Lstr_bstdlib_0]
    mov rdi, rax
    call _printf_pc_vargs
    mov eax, 1
    mov edi, eax
    call _exit_i32
    jmp .end_.if_stmt_44
.end_.if_stmt_44:
    mov rax, qword [__heap]
    mov qword [rbp - 40], rax
.while_loop_12:
    mov rax, qword [rbp - 40]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_4
    mov rbx, qword [rbp - 40]
    mov rcx, qword [rbp - 8]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
    jz .skip_right_5
    mov rcx, qword [rbp - 40]
    mov rcx, qword [rcx + 24]
    mov rdx, qword [rbp - 40]
    add rdx, rcx
    mov rcx, qword [rbp - 8]
    cmp rdx, rcx
    setg dl
    test dl, dl
    setne dl
.skip_right_5:
    and bl, dl
    test bl, bl
    setz cl
    test cl, cl
    setne cl
.skip_right_4:
    and al, cl
    cmp al, 0
    je .end_.while_loop_12
    mov rax, qword [rbp - 40]
    mov rax, qword [rax]
    lea rbx, [rbp - 40]
    mov [rbx], rax 
    jmp .while_loop_12

.end_.while_loop_12:
    mov rax, qword [null]
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 40]
    mov rax, qword [rax + 8]
    mov qword [rbp - 56], rax
.while_loop_13:
    mov rax, qword [rbp - 56]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_6
    mov rbx, qword [rbp - 56]
    mov rcx, qword [__HEADER_SIZE]
    add rbx, rcx
    mov rcx, qword [rbp - 8]
    cmp rbx, rcx
    setl bl
    test bl, bl
    setne bl
.skip_right_6:
    and al, bl
    cmp al, 0
    je .end_.while_loop_13
    mov rax, qword [rbp - 56]
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    mov rax, qword [rbp - 56]
    mov rax, qword [rax]
    lea rbx, [rbp - 56]
    mov [rbx], rax 
    jmp .while_loop_13

.end_.while_loop_13:
    mov rax, qword [rbp - 8]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    mov rax, qword [rbp - 56]
    mov rdx, rax
    call ___createFreeBlock_p_u64_p__FreeBlock
    mov qword [rbp - 64], rax
.if_stmt_45:
    mov rax, qword [rbp - 48]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_8
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 40]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_45
.else_stmt_8:
    mov rax, qword [rbp - 64]
    mov rbx, qword [rbp - 48]
    mov [rbx], rax 
.end_.if_stmt_45:
    mov rax, qword [rbp - 48]
    mov rdi, rax
    mov rax, qword [rbp - 64]
    mov rsi, rax
    call ___tryMergeFreeBlocks_p__FreeBlock_p__FreeBlock
    mov rax, qword [rbp - 8]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 72], rax
    mov rax, qword [__MAGIC_FREED]
    mov rbx, qword [rbp - 72]
    mov qword [rbx + 8], rax 
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    add [rbx + 16], rax 
.ret_from_free_p:
    add rsp, 72
    pop rbp
    ret
___reallocShrink_p_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 88
    mov qword [rbp - 8], rdi
    mov qword [rbp - 16], rsi
    mov rax, qword [rbp - 8]
    mov rbx, qword [__TOTSIZE_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 24], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 32], rax
    mov rax, qword [rbp - 16]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [__MAX_ALIGN_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov qword [rbp - 40], rax
    mov rax, qword [__heap]
    mov qword [rbp - 48], rax
.while_loop_14:
    mov rax, qword [rbp - 48]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_7
    mov rbx, qword [rbp - 48]
    mov rcx, qword [rbp - 8]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
    jz .skip_right_8
    mov rcx, qword [rbp - 48]
    mov rcx, qword [rcx + 24]
    mov rdx, qword [rbp - 48]
    add rdx, rcx
    mov rcx, qword [rbp - 8]
    cmp rdx, rcx
    setg dl
    test dl, dl
    setne dl
.skip_right_8:
    and bl, dl
    test bl, bl
    setz cl
    test cl, cl
    setne cl
.skip_right_7:
    and al, cl
    cmp al, 0
    je .end_.while_loop_14
    mov rax, qword [rbp - 48]
    mov rax, qword [rax]
    lea rbx, [rbp - 48]
    mov [rbx], rax 
    jmp .while_loop_14

.end_.while_loop_14:
.if_stmt_46:
    mov rax, qword [rbp - 32]
    mov rbx, qword [rbp - 16]
    sub rax , rbx
    mov rbx, qword [__HEADER_SIZE]
    mov rcx, qword [__MAX_ALIGN_SIZE]
    add rbx, rcx
    cmp rax, rbx
    setge al
    cmp al, 0
    je .end_.if_stmt_46
    mov rax, qword [null]
    mov qword [rbp - 56], rax
    mov rax, qword [rbp - 48]
    mov rax, qword [rax + 8]
    mov qword [rbp - 64], rax
.while_loop_15:
    mov rax, qword [rbp - 64]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_9
    mov rbx, qword [rbp - 64]
    mov rcx, qword [__HEADER_SIZE]
    add rbx, rcx
    mov rcx, qword [rbp - 8]
    cmp rbx, rcx
    setl bl
    test bl, bl
    setne bl
.skip_right_9:
    and al, bl
    cmp al, 0
    je .end_.while_loop_15
    mov rax, qword [rbp - 64]
    lea rbx, [rbp - 56]
    mov [rbx], rax 
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    lea rbx, [rbp - 64]
    mov [rbx], rax 
    jmp .while_loop_15

.end_.while_loop_15:
    mov rax, qword [rbp - 8]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    sub rax , rbx
    mov rsi, rax
    mov rax, qword [rbp - 64]
    mov rdx, rax
    call ___createFreeBlock_p_u64_p__FreeBlock
    mov qword [rbp - 72], rax
.if_stmt_47:
    mov rax, qword [rbp - 56]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_9
    mov rax, qword [rbp - 72]
    mov rbx, qword [rbp - 48]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_47
.else_stmt_9:
    mov rax, qword [rbp - 72]
    mov rbx, qword [rbp - 56]
    mov [rbx], rax 
.end_.if_stmt_47:
    mov rax, qword [rbp - 56]
    mov rdi, rax
    mov rax, qword [rbp - 72]
    mov rsi, rax
    call ___tryMergeFreeBlocks_p__FreeBlock_p__FreeBlock
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 40]
    sub rax , rbx
    mov rbx, qword [rbp - 48]
    add [rbx + 16], rax 
    jmp .end_.if_stmt_46
.end_.if_stmt_46:
    mov rax, qword [rbp - 8]
    mov rbx, qword [__REQSIZE_OFFSET]
    sub rax , rbx
    mov qword [rbp - 80], rax
    mov rax, qword [rbp - 8]
    mov rbx, qword [__TOTSIZE_OFFSET]
    sub rax , rbx
    mov qword [rbp - 88], rax
    mov rax, qword [rbp - 16]
    lea rbx, [rbp - 80]
    mov rbx, qword [rbx]
    mov qword [rbx], rax 
    mov rax, qword [rbp - 40]
    lea rbx, [rbp - 88]
    mov rbx, qword [rbx]
    mov qword [rbx], rax 
    mov rax, qword [rbp - 8]
    jmp .ret_from___reallocShrink_p_u64
.ret_from___reallocShrink_p_u64:
    add rsp, 88
    pop rbp
    ret
___reallocExtendInPlace_p_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 120
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov rax, qword [rbp - 16]
    mov rbx, qword [__TOTSIZE_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 32], rax
    mov rax, qword [rbp - 32]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [__MAX_ALIGN_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 16]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 56], rax
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 40]
    add rax, rbx
    mov qword [rbp - 64], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [__MAX_ALIGN_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 72], rax
    mov rax, qword [__heap]
    mov qword [rbp - 80], rax
.while_loop_16:
    mov rax, qword [rbp - 80]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_10
    mov rbx, qword [rbp - 80]
    mov rcx, qword [rbp - 16]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
    jz .skip_right_11
    mov rcx, qword [rbp - 80]
    mov rcx, qword [rcx + 24]
    mov rdx, qword [rbp - 80]
    add rdx, rcx
    mov rcx, qword [rbp - 16]
    cmp rdx, rcx
    setg dl
    test dl, dl
    setne dl
.skip_right_11:
    and bl, dl
    test bl, bl
    setz cl
    test cl, cl
    setne cl
.skip_right_10:
    and al, cl
    cmp al, 0
    je .end_.while_loop_16
    mov rax, qword [rbp - 80]
    mov rax, qword [rax]
    lea rbx, [rbp - 80]
    mov [rbx], rax 
    jmp .while_loop_16

.end_.while_loop_16:
    mov rax, qword [null]
    mov qword [rbp - 88], rax
    mov rax, qword [rbp - 80]
    mov rax, qword [rax + 8]
    mov qword [rbp - 96], rax
.while_loop_17:
    mov rax, qword [rbp - 96]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_12
    mov rbx, qword [rbp - 96]
    mov rcx, qword [rbp - 64]
    cmp rbx, rcx
    setne bl
    test bl, bl
    setne bl
.skip_right_12:
    and al, bl
    cmp al, 0
    je .end_.while_loop_17
    mov rax, qword [rbp - 96]
    lea rbx, [rbp - 88]
    mov [rbx], rax 
    mov rax, qword [rbp - 96]
    mov rax, qword [rax]
    lea rbx, [rbp - 96]
    mov [rbx], rax 
    jmp .while_loop_17

.end_.while_loop_17:
.if_stmt_48:
    mov rax, qword [rbp - 96]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_48
    lea rax, [Lstr_bstdlib_1]
    mov rdi, rax
    call _printf_pc_vargs
    mov eax, 1
    mov edi, eax
    call _exit_i32
    jmp .end_.if_stmt_48
.end_.if_stmt_48:
.if_stmt_49:
    mov rax, qword [rbp - 88]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_10
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 80]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_49
.else_stmt_10:
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rbx, qword [rbp - 88]
    mov [rbx], rax 
.end_.if_stmt_49:
    mov rax, qword [rbp - 56]
    mov rax, qword [rax + 16]
    mov qword [rbp - 104], rax
    mov rax, qword [rbp - 64]
    mov rax, qword [rax + 16]
    mov rbx, qword [rbp - 72]
    mov rcx, qword [rbp - 40]
    sub rbx , rcx
    sub rax , rbx
    mov qword [rbp - 112], rax
.if_stmt_50:
    mov rax, qword [__HEADER_SIZE]
    mov rbx, qword [__MAX_ALIGN_SIZE]
    add rax, rbx
    mov rbx, qword [rbp - 112]
    cmp rbx, rax
    setge bl
    cmp bl, 0
    je .end_.if_stmt_50
    mov rax, qword [rbp - 56]
    mov rbx, qword [rbp - 48]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [rbp - 112]
    mov rsi, rax
    mov rax, qword [rbp - 64]
    mov rax, qword [rax]
    mov rdx, rax
    call ___createFreeBlock_p_u64_p__FreeBlock
    mov qword [rbp - 120], rax
.if_stmt_51:
    mov rax, qword [rbp - 88]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .else_stmt_11
    mov rax, qword [rbp - 120]
    mov rbx, qword [rbp - 80]
    mov [rbx + 8], rax 
    jmp .end_.if_stmt_51
.else_stmt_11:
    mov rax, qword [rbp - 120]
    mov rbx, qword [rbp - 88]
    mov [rbx], rax 
.end_.if_stmt_51:
    jmp .end_.if_stmt_50
.end_.if_stmt_50:
    mov rax, qword [rbp - 48]
    mov rbx, qword [rbp - 56]
    mov qword [rbx + 16], rax 
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 56]
    mov qword [rbx + 24], rax 
    mov rax, qword [rbp - 48]
    mov rbx, qword [rbp - 32]
    sub rax , rbx
    mov rbx, qword [rbp - 80]
    sub [rbx + 16], rax 
    mov rax, qword [rbp - 16]
    jmp .ret_from___reallocExtendInPlace_p_u64
.ret_from___reallocExtendInPlace_p_u64:
    add rsp, 120
    pop rbp
    ret
___reallocFallback_p_u64_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 40
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov qword [rbp - 32], rdx
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 40]
    mov rdi, rax
    mov rax, qword [rbp - 16]
    mov rsi, rax
    mov rax, qword [rbp - 32]
    mov rdx, rax
    call _memcpy_p_p_u64
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _free_p
    mov rax, qword [rbp - 40]
    jmp .ret_from___reallocFallback_p_u64_u64
.ret_from___reallocFallback_p_u64_u64:
    add rsp, 40
    pop rbp
    ret
_realloc_p_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 88
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
    mov rax, qword [rbp - 16]
    mov rbx, qword [__MAGIC_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 32], rax
.if_stmt_52:
    mov rax, qword [rbp - 32]
    mov rbx, qword [__MAGIC_FREED]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_52
    lea rax, [Lstr_bstdlib_2]
    mov rdi, rax
    call _printf_pc_vargs
    mov eax, 1
    mov edi, eax
    call _exit_i32
    jmp .end_.if_stmt_52
.end_.if_stmt_52:
.if_stmt_53:
    mov rax, qword [rbp - 16]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_53
    mov rax, qword [rbp - 24]
    mov rdi, rax
    call _malloc_u64
    jmp .ret_from_realloc_p_u64
    jmp .end_.if_stmt_53
.end_.if_stmt_53:
.if_stmt_54:
    mov rax, qword [rbp - 24]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_54
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _free_p
    mov rax, qword [null]
    jmp .ret_from_realloc_p_u64
    jmp .end_.if_stmt_54
.end_.if_stmt_54:
    mov rax, qword [rbp - 16]
    mov rbx, qword [__TOTSIZE_OFFSET]
    sub rax , rbx
    mov rax, qword [rax]
    mov qword [rbp - 40], rax
    mov rax, qword [rbp - 40]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 48], rax
    mov rax, qword [rbp - 24]
    mov rbx, qword [__HEADER_SIZE]
    add rax, rbx
    mov rdi, rax
    mov rax, qword [__MAX_ALIGN_SIZE]
    mov rsi, rax
    call ___align_to_u64_u64
    mov qword [rbp - 56], rax
.if_stmt_55:
    mov rax, qword [rbp - 24]
    mov rbx, qword [rbp - 48]
    cmp rax, rbx
    setb al
    cmp al, 0
    je .end_.if_stmt_55
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    call ___reallocShrink_p_u64
    jmp .ret_from_realloc_p_u64
    jmp .end_.if_stmt_55
.end_.if_stmt_55:
    mov rax, qword [rbp - 16]
    mov rbx, qword [__HEADER_SIZE]
    sub rax , rbx
    mov qword [rbp - 64], rax
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 48]
    add rax, rbx
    mov qword [rbp - 72], rax
    mov rax, qword [__heap]
    mov qword [rbp - 80], rax
    mov rax, qword [__heap]
    mov qword [rbp - 88], rax
.while_loop_18:
    mov rax, qword [rbp - 80]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    test al, al
    setne al
    jz .skip_right_13
    mov rbx, qword [rbp - 80]
    mov rcx, qword [rbp - 16]
    cmp rbx, rcx
    setb bl
    test bl, bl
    setne bl
    jz .skip_right_14
    mov rcx, qword [rbp - 80]
    mov rcx, qword [rcx + 24]
    mov rdx, qword [rbp - 80]
    add rdx, rcx
    mov rcx, qword [rbp - 16]
    cmp rdx, rcx
    setg dl
    test dl, dl
    setne dl
.skip_right_14:
    and bl, dl
    test bl, bl
    setz cl
    test cl, cl
    setne cl
.skip_right_13:
    and al, cl
    cmp al, 0
    je .end_.while_loop_18
    mov rax, qword [__heap]
    lea rbx, [rbp - 88]
    mov [rbx], rax 
    mov rax, qword [rbp - 80]
    mov rax, qword [rax]
    lea rbx, [rbp - 80]
    mov [rbx], rax 
    jmp .while_loop_18

.end_.while_loop_18:
.if_stmt_56:
    mov rax, qword [rbp - 80]
    mov rbx, qword [null]
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_56
    lea rax, [Lstr_bstdlib_3]
    mov rdi, rax
    call _printf_pc_vargs
    lea rax, [Lstr_bstdlib_4]
    mov rdi, rax
    mov rax, qword [rbp - 16]
    push rax
    sub rsp, 8
    call _printf_pc_vargs
    add rsp, 16
    lea rax, [Lstr_bstdlib_5]
    mov rdi, rax
    mov rax, qword [rbp - 88]
    push rax
    mov rax, qword [rbp - 88]
    mov rax, qword [rax + 24]
    mov rbx, qword [rbp - 88]
    add rbx, rax
    push rbx
    call _printf_pc_vargs
    add rsp, 16
    mov eax, 1
    mov edi, eax
    call _exit_i32
    jmp .end_.if_stmt_56
.end_.if_stmt_56:
.if_stmt_57:
    mov rax, qword [rbp - 80]
    mov rax, qword [rax + 24]
    mov rbx, qword [rbp - 80]
    add rbx, rax
    mov rax, qword [rbp - 72]
    cmp rax, rbx
    setge al
    cmp al, 0
    je .end_.if_stmt_57
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    mov rax, qword [rbp - 48]
    mov rdx, rax
    call ___reallocFallback_p_u64_u64
    jmp .ret_from_realloc_p_u64
    jmp .end_.if_stmt_57
.end_.if_stmt_57:
.if_stmt_58:
    mov rax, qword [rbp - 72]
    mov rax, qword [rax + 8]
    mov rbx, qword [__MAGIC_FREED]
    cmp rax, rbx
    sete al
    test al, al
    setne al
    jz .skip_right_15
    push rax
    mov rax, qword [rbp - 80]
    mov rdi, rax
    mov rax, qword [rbp - 72]
    mov rsi, rax
    sub rsp, 8
    call ___isBlockInFreeList_p__Heap_p__FreeBlock
    add rsp, 8
    mov bl, al
    pop rax
    test bl, bl
    setne bl
.skip_right_15:
    and al, bl
    test al, al
    setne al
    jz .skip_right_16
    mov rbx, qword [rbp - 72]
    mov rbx, qword [rbx + 16]
    mov rcx, qword [rbp - 40]
    add rcx, rbx
    mov rbx, qword [rbp - 56]
    cmp rcx, rbx
    setge cl
    test cl, cl
    setne cl
.skip_right_16:
    and al, cl
    cmp al, 0
    je .end_.if_stmt_58
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    call ___reallocExtendInPlace_p_u64
    jmp .ret_from_realloc_p_u64
    jmp .end_.if_stmt_58
.end_.if_stmt_58:
    mov rax, qword [rbp - 16]
    mov rdi, rax
    mov rax, qword [rbp - 24]
    mov rsi, rax
    mov rax, qword [rbp - 48]
    mov rdx, rax
    call ___reallocFallback_p_u64_u64
    jmp .ret_from_realloc_p_u64
.ret_from_realloc_p_u64:
    add rsp, 88
    pop rbp
    ret
_zalloc_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov rax, qword [rbp - 16]
    mov rdi, rax
    call _malloc_u64
    mov qword [rbp - 24], rax
.if_stmt_59:
    mov rax, qword [rbp - 24]
    mov rbx, qword [null]
    cmp rax, rbx
    setne al
    cmp al, 0
    je .end_.if_stmt_59
    mov rax, qword [rbp - 24]
    mov rdi, rax
    mov al, 0
    mov sil, al
    mov rax, qword [rbp - 16]
    mov rdx, rax
    call _memset_p_c_u64
    jmp .end_.if_stmt_59
.end_.if_stmt_59:
    mov rax, qword [rbp - 24]
    jmp .ret_from_zalloc_u64
.ret_from_zalloc_u64:
    add rsp, 24
    pop rbp
    ret
