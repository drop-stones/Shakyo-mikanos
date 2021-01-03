; asmfunc.asm
; 
; System V AMD64 Calling Convention
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

global IoOut32  ; void IoOut32(uint16_t addr, uint32_t data);
IoOut32:
    mov dx, di      ; dx = addr
    mov eax, esi    ; eax = data
    out dx, eax     ; usage: out <addr>, <data>
    ret

global IoIn32   ; uint32_t IoIn32(uint16_t addr);
IoIn32:
    mov dx, di      ; dx = addr
    in eax, dx      ; usage: in <dest>, <addr>
    ret

global GetCS    ; uint16_t GetCS(void)
GetCS:
    xor eax, eax    ; also clears upper 32 bits of rax
    mov ax, cs
    ret

global LoadIDT  ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di       ; [rsp] = limit
    mov [rsp + 2], rsi  ; [rsp + 2] = offset
    lidt [rsp]          ; 10 bytes
    mov rsp, rbp
    pop rbp
    ret