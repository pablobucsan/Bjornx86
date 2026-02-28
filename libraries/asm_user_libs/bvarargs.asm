
section .text

global ___get_rbp
global _varg_start_u64


___get_rbp:
    mov rax, rbp
    ret


; varg_start(uint64 count) --> ptr<void> 
; Cant call nothing because that'll push and mess the calculation
_varg_start_u64:
    mov rax, rbp
    mov rbx, 1
    and rbx, rdi     ; rbx = count & 1 to see if its odd 
    test rbx, rbx 
    jz _count_even
    imul rdi, rdi, 8
    add rdi, 16
    add rax, rdi     ; rax = rbp + count * 8 + 16 
    ret
_count_even:
    imul rdi, rdi, 8  ; rdi = count * 8
    add rdi, 8        ; rdi = count * 8 + 8
    add rax, rdi      ; rax = rax + rdi = rbp + count * 8 + 8
    ret


