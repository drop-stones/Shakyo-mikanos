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

global LoadGDT  ; void LoadGDT(uint16_t limit, uint64_t offset);
LoadGDT:
    push rbp
    mov rbp, rsp
    sub rsp, 10
    mov [rsp], di       ; limit
    mov [rsp + 2],  rsi ; offset
    lgdt [rsp]
    mov rsp, rbp
    pop rbp
    ret

global SetCSSS  ; void SetCSSS(uint16_t cs, uint16_t ss);
SetCSSS:
    push rbp
    mov rbp, rsp
    mov ss, si      ; set ss
    mov rax, .next
    push rdi        ; CS
    push rax        ; RIP
    o64 retf        ; far return: pop "return-addr" + "cs", o64: 32bit -> 64bit
.next:
    mov rsp, rbp
    pop rbp
    ret

global SetDSAll ; void SetDSAll(uint16_t value);
SetDSAll:
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

global SetCR3   ; void SetCR3(uint64_t value);
SetCR3:
    mov cr3, rdi
    ret

global ShutdownBIOS ; void ShutdownBIOS();
ShutdownBIOS:
    mov ax, 0x1000
    mov ax, ss
    mov sp, 0xf000
    mov ax, 0x5307
    mov bx, 0x0001
    mov cx, 0x0003
    int 0x15
    ret

global ShutdownAPM  ; void ShutdownAPM();
ShutdownAPM:
    ; connect to APM API
    mov ax, 0x5301
    xor bx, bx
    int 0x15
    ; try to set APM version
    mov ax, 0x530e
    xor bx, bx
    mov cx, 0x102
    int 0x15
    ; turn off the system
    mov ax, 0x5307
    mov bx, 0x0001
    mov cx, 0x0003
    int 0x15
    ret

global ShutdownQEMU ; void ShutdownQEMU();
ShutdownQEMU:
    xor eax, eax
    mov edx, 0x4004
    mov ax, 0
    or ax, 0b1000000000000
    out dx, ax
    ;mov dx, 0x604
    ;mov eax, 0x2000
    ;out dx, eax
    ;ret

; set_main_stack
extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
    mov rsp, kernel_main_stack + 1024 * 1024
    call KernelMainNewStack
.fin:
    hlt
    jmp .fin