section .text

; Defines the following functions

;Printing and writing
global _print_u8
global _print_u16 
global _print_u32
global _print_i8
global _print_i16 
global _print_i32
global _putc_c
global _write_u32_s_u32

; File access
global _open_s_u32_u32
global _close_u32



;
;
;
; Printing and writing


; Function: print(uint8 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_u8:
    push rbp
    mov rbp, rsp
    sub rsp, 8          ; Buffer for string (up to 4 bytes: 3 digits + newline). Gotta align to 16 bytes. buffer[8]

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 7          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = [?,?,?,'\n'] 

    ; Convert uint8 in dil to string
    movzx rax, dil      ; Zero-extend uint8 to 64-bit
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 8          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret


; Function: print(int8 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_i8:
    push rbp
    mov rbp, rsp
    sub rsp, 8          ; Buffer for string (up to 5 bytes: '-' + 3 digits + newline). Gotta align to 16 bytes. buffer[8]

    ; Check if negative
    movsx rax, dil        ; sign extend dil into rax for proper negation
    mov rbx, rax
    cmp dil, 0            ; Compare with 0, if bigger is positive 
    jge .skip_negative    ; Skip printing '-' 

    ; Print '-'
    mov rsi, rsp        ; Point to start of buffer 
    mov byte [rsi], 45  ; Print '-'. buffer = ['-',?,?,?,?,?,?,?]
    mov rax, 1 
    mov rdi, 1 
    mov rdx, 1 
    syscall
   
    mov rax, rbx
    neg rax 

.skip_negative:
    ; Convert int8 in dil to string
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 7          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = ['-',?,?,?,?,?,?,'\n'] 
    

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 8          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret




; Function: print(uint16 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_u16:
    push rbp
    mov rbp, rsp
    sub rsp, 8          ; Buffer for string (up to 6 bytes: 5 digits + newline). Gotta align to 16 bytes. buffer[8]

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 7          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = [?,?,?,'\n'] 

    ; Convert uint16 in di to string
    movzx rax, di      ; Zero-extend uint16 to 64-bit
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 8          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret


; Function: print(int16 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_i16:
    push rbp
    mov rbp, rsp
    sub rsp, 8          

    ; Check if negative
    movsx rax, di        ; sign extend dil into rax for proper negation
    mov rbx, rax         ; save rax for later 
    cmp di, 0            ; Compare with 0, if bigger is positive 
    jge .skip_negative    ; Skip printing '-' 

    ; Print '-'
    mov rsi, rsp        ; Point to start of buffer 
    mov byte [rsi], 45  ; Print '-'. buffer = ['-',?,?,?,?,?,?,?]
    mov rax, 1 
    mov rdi, 1 
    mov rdx, 1 
    syscall
   
    mov rax, rbx
    neg rax 

.skip_negative:
    ; Convert int8 in dil to string
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 7          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = ['-',?,?,?,?,?,?,'\n'] 
    

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 8          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret



; Function: print(uint32 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_u32:
    push rbp
    mov rbp, rsp
    sub rsp, 24          ; Buffer for string (up to 11 bytes: 10 digits + newline). Gotta align to 16 bytes. buffer[24]

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 23          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = [?,?,?,'\n'] 

    ; Convert uint32 in edi to string
    mov eax, edi      ; 
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 24          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret



; Function: print(int32 value). Convert value to "value\n"
; Prints value as decimal number followed by newline to stdout
_print_i32:
    push rbp
    mov rbp, rsp
    sub rsp, 24          ; Buffer for string (up to 5 bytes: '-' + 3 digits + newline). Gotta align to 16 bytes. buffer[8]

    ; Check if negative
    movsx rax, edi        ; sign extend dil into rax for proper negation
    mov rbx, rax
    cmp edi, 0            ; Compare with 0, if bigger is positive 
    jge .skip_negative    ; Skip printing '-' 

    ; Print '-'
    mov rsi, rsp        ; Point to start of buffer 
    mov byte [rsi], 45  ; Print '-'. buffer = ['-',?,?,?,?,?,?,?]
    mov rax, 1 
    mov rdi, 1 
    mov rdx, 1 
    syscall
   
    mov rax, rbx
    neg rax 

.skip_negative:
    ; Convert int8 in dil to string
    mov rcx, 10         ; Base 10 divisor -> For converting the number to decimal digits

    ; Initialize buffer
    mov rsi, rsp        ; Point to start of buffer that will hold "value\n" 
    add rsi, 23          ; Move to end of buffer (for reverse writing)
    mov byte [rsi], 10  ; Store newline at the end. buffer = ['-',?,?,?,?,?,?,'\n'] 
    

    ; Convert number to ASCII (write digits in reverse)
.convert_loop:
    dec rsi             ; Move buffer pointer backward
    xor rdx, rdx        ; Clear for division
    div rcx             ; rax = quotient, rdx = remainder. 42 / 10 => quotient 4, remainder 2
    add dl, '0'         ; Convert remainder to ASCII => 2 -> '2'
    mov [rsi], dl       ; Store digit ->  buffer = [?,?,'2','\n']
    test rax, rax       ; Check if quotient is 0 to determine if more digits need processing
    jnz .convert_loop   ; Continue if quotient != 0

    ; Syscall: write(1, rsi, length)
    mov rax, 1          ; Syscall: write
    mov rdi, 1          ; File descriptor: stdout
    mov rdx, rsp        ; Calculate length
    add rdx, 24          ; End of buffer
    sub rdx, rsi        ; Length = end - start
    syscall

    leave               ; Restore stack (mov rsp, rbp; pop rbp)
    ret



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
    lea rsi, [rbp - 8]         ; Buffer

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