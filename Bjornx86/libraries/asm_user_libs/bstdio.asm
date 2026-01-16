section .text

; Defines the following functions

;Printing and writing
global _putc_c
global _write_u32_s_u32

; File access
global _open_s_u32_u32
global _close_u32



; 27/11
; Function putc(char c). Print the ASCII value
; Prints ASCII (no new line)
_putc_c:
    push rbp
    mov rbp, rsp
    sub rsp, 8 

    mov byte [rbp - 8], dil     ; Save the char

    ;RDI has the 1st argument. In this case, DIL since its only 1 byte
    mov rax, 1                  ; Syscall: write
    mov rdi, 1                  ; File descriptor: stdout
    mov rdx, 1                  ; Length: Character
    lea rsi, [rbp - 8]          ; Buffer

    syscall 

    add rsp, 8
    pop rbp 
    ret

; Function write(uint32 fd, str s, uint32 count)
; Prints a buffer to fd without new line
_write_u32_s_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 8

    mov rax, 1           ; Syscall: write

    syscall

    add rsp, 8
    pop rbp
    ret

;
;
;
;
; Accessing files

;Function open(str filePath, uint32 open_flags, uint32 mode)
;Opens the file specified at the filepath with the provided flags. 
;Returns a file descriptor uint32 to reference such file
_open_s_u32_u32:
    push rbp
    mov rbp, rsp 
    sub rsp, 8

    mov rax, 2                   ; Syscall to open

    syscall

    add rsp, 8
    pop rbp 
    ret

; Function close(uint32 fd)
; Closes the file linked to the provided file descriptor
_close_u32:
    push rbp 
    mov rbp, rsp 
    sub rsp, 8

    mov rax, 3                     ; Syscall to close
    
    syscall

    add rsp, 8
    pop rbp
    ret