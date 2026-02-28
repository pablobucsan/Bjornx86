section .text

; Defines the following functions



; file descriptor fd needs to be int32 because it can be negative for error purposes
;Printing and writing
global _putc_c
global _fputc_i32_c
global _write_i32_pc_u32

; File access
global _open_pc_u16_u16
global _close_i32
global _read_i32_pc_u32

; Directory create
global _mkdir_pc_u16



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


_fputc_i32_c:
    push rbp
    mov rbp, rsp
    sub rsp, 8 

    mov byte [rbp - 8], sil     ; Save the char

    mov rax, 1                  ; Syscall : Write
    ;movsxd rdi, edi                            ; RDI has the file descriptor
    mov rdx, 1                  ; Length  : Character
    lea rsi, [rbp - 8]          ; Buffer


    syscall 

    add rsp, 8
    pop rbp 
    ret
    

; Function read(int32 fd, ptr<char> buf, uint32 count)
; Reads "count" chars and puts them into "buf"
_read_i32_pc_u32:
    mov rax, 0
    syscall 
    ret

; Function write(int32 fd, str s, uint32 count)
; Prints a buffer to fd without new line
_write_i32_pc_u32:

    mov rax, 1           ; Syscall: write

    syscall

    ret


;
; Accessing files

;Function open(str filePath, uint16 open_flags, uint16 mode)
;Opens the file specified at the filepath with the provided flags. 
;Returns a file descriptor uint32 to reference such file
_open_pc_u16_u16:

    movzx rsi, si
    movzx rdx, dx
    mov rax, 2                   ; Syscall to open

    syscall

    ret

; Function close(int32 fd)
; Closes the file linked to the provided file descriptor
_close_i32:

    mov rax, 3                     ; Syscall to close
    
    syscall

    ret

; Function mkdir(str pathname, uint16 mode)
_mkdir_pc_u16:
    mov rax, 83
    syscall 
    ret