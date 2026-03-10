section .data
section .text
_abs_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
    mov rax, qword [rbp - 8]
    jmp .ret_from_abs_u64
.ret_from_abs_u64:
    add rsp, 8
    pop rbp
    ret
_abs_i64:
    push rbp
    mov rbp, rsp
    sub rsp, 8
    mov qword [rbp - 8], rdi
.if_stmt_0:
    mov rax, qword [rbp - 8]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_0
    mov rax, qword [rbp - 8]
    neg rax
    jmp .ret_from_abs_i64
    jmp .end_.if_stmt_0
.end_.if_stmt_0:
    mov rax, qword [rbp - 8]
    jmp .ret_from_abs_i64
.ret_from_abs_i64:
    add rsp, 8
    pop rbp
    ret
_digitsof_i64_u64:
    push rbp
    mov rbp, rsp
    sub rsp, 24
    mov qword [rbp - 16], rdi
    mov qword [rbp - 24], rsi
.if_stmt_1:
    mov rax, qword [rbp - 16]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setl al
    cmp al, 0
    je .end_.if_stmt_1
    mov rax, qword [rbp - 16]
    neg rax
    lea rbx, [rbp - 16]
    mov qword [rbx], rax 
    jmp .end_.if_stmt_1
.end_.if_stmt_1:
.if_stmt_2:
    mov rax, qword [rbp - 16]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    sete al
    cmp al, 0
    je .end_.if_stmt_2
    mov al, 1
    jmp .ret_from_digitsof_i64_u64
    jmp .end_.if_stmt_2
.end_.if_stmt_2:
    mov al, 0
    mov byte [rbp - 8], al
.while_loop_0:
    mov rax, qword [rbp - 16]
    mov bl, 0
    movzx rbx, bl
    cmp rax, rbx
    setg al
    cmp al, 0
    je .end_.while_loop_0
    lea rax, [rbp - 8]
    mov bl, byte [rax]
    inc byte [rax] 
    mov rax, qword [rbp - 16]
    mov rbx, qword [rbp - 24]
    cqo
    idiv rbx
    lea rbx, [rbp - 16]
    mov qword [rbx], rax 
    jmp .while_loop_0

.end_.while_loop_0:
    mov al, byte [rbp - 8]
    jmp .ret_from_digitsof_i64_u64
.ret_from_digitsof_i64_u64:
    add rsp, 24
    pop rbp
    ret
