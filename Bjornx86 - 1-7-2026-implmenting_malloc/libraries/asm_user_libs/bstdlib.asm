section .text


global _mmap_p_u64_u64_u64_u64_u64



; Function mmap(ptr<void> addr, uint64 len, uint64 prot, uint64 fd, uint64 off)
; Wrapper for syscall 9 ---> puts in RAX the address pointed to the freshly memory mapped block
_mmap_p_u64_u64_u64_u64_u64:
    push rbp
    mov rbp, rsp 
    sub rsp,8 

    
    mov rax, 9          ; Syscall 9
    mov r10, rcx        ; 4th argument is expected in r10 as per syscalls, not in rcx as system v abi convention demands  

    syscall 

    add rsp, 8
    pop rbp 
    ret

