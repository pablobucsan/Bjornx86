
section .data
; STDOUT_struct
STDOUT_struct:
    dq 0         ; stdout->path
    dq 0         ; stdout->pos
    dd 1         ; stdout->fd
    dd 1         ; stdout->open_flags
    dd 0         ; stdout->mode

STDIN_struct:
    dq 0         ; stdin->path
    dq 0         ; stdin->pos
    dd 0         ; stdin->fd
    dd 1         ; stdin->open_flags
    dd 0         ; stdin->mode

STDOUT: dq STDOUT_struct

STDIN: dq STDIN_struct



section .text
_bjorn_ctrl_start:
    ; Set up argc and argv
    mov rbp, rsp
    mov rdi, qword [rbp]
    lea rsi, qword [rbp + 8]
    and rsp, -16 
    ; Call user's main()
    call _main_u64_ppc

    ; Exit 
    mov rax, 60
    mov rdi, 0
    syscall

