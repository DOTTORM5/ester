global start
extern long_mode_start

section .text
bits 32
start:
    mov esp, stack_top

    ; check multiboot2 presence
    cmp eax, 0x36d76289
    jne .no_multiboot
    

    ; Check if CPUID is supported by attempting to flip the ID bit (bit 21)
    ; in the FLAGS register. If we can flip it, CPUID is available.

    ; Copy FLAGS in to EAX via stack
    pushfd
    pop eax

    ; Copy to ECX as well for comparing later on
    mov ecx, eax

    ; Flip the ID bit
    xor eax, 1 << 21

    ; Copy EAX to FLAGS via the stack
    push eax
    popfd

    ; Copy FLAGS back to EAX (with the flipped bit if CPUID is supported)
    pushfd
    pop eax

    ; Restore FLAGS from the old version stored in ECX (i.e. flipping the
    ; ID bit back if it was ever flipped).
    push ecx
    popfd

    ; Compare EAX and ECX. If they are equal then that means the bit
    ; wasn't flipped, and CPUID isn't supported.
    cmp eax, ecx
    je .no_cpuid

    ; test if extended processor info in available
    mov eax, 0x80000000    ; implicit argument for cpuid
    push ebx               ; save ebx because cpuid overwrite it, i need this for the multiboot2 info struct address
    cpuid                  ; get highest supported argument
    cmp eax, 0x80000001    ; it needs to be at least 0x80000001
    jb .no_long_mode       ; if it's less, the CPU is too old for long mode

    ; use extended info to test if long mode is available
    mov eax, 0x80000001    ; argument for extended processor info
    cpuid                  ; returns various feature bits in ecx and edx
    test edx, 1 << 29      ; test if the LM-bit is set in the D-register
    jz .no_long_mode       ; If it's not set, there is no long mode

    pop ebx                ; restore ebx

    ; SETUP PAGE TABLE
    ; map first P4 entry to P3 table
    mov eax, p3_table
    or eax, 0b11           ; present + writable
    mov [p4_table], eax

    ; map first P3 entry to P2_0 table
    mov eax, p2_table_0
    or eax, 0b11           ; present + writable
    mov [p3_table], eax

    ; map each P2 entry to a huge 2MiB page
    mov ecx, 0         ; counter variable

.map_p2_table_0:
    ; map ecx-th P2 entry to a huge page that starts at address 2MiB*ecx
    mov eax, 0x200000                      ; 2MiB
    mul ecx                                ; start address of ecx-th page
    or eax, 0b10000011                     ; present + writable + huge
    mov [p2_table_0 + ecx * 8], eax        ; map ecx-th entry
    inc ecx                                ; increase counter
    cmp ecx, 512                           ; if counter == 512, the whole P2 table is mapped
    jne .map_p2_table_0                    ; else map the next entry

    mov ecx, 0


    ;Now map the specific address 0xFEBB1000
    ; Step 1: Map P3 entry for the 0xF0000000 range to a new P2 table
    mov eax, p2_table_1
    or eax, 0b11           ; Present + Writable
    mov [p3_table+24], eax   ; This maps the 0xF0000000 range to P2_1

    ; Step 2: Map P2 entry for 0xFEBB1000 to a P1 table
    mov eax, p1_table
    or eax, 0b11           ; Present + Writable
    mov [p2_table_1+8*0x1F5], eax   ; Map the correct P2 entry

    ; Step 3: Map P1 entry for 0xFEBB1000 to the physical address
    mov eax, 0xFEBB1000        ; Physical address of the AHCI BAR5
    or eax, 0b11               ; Present + Writable
    mov [p1_table + 8*(0x1B1)], eax  ; Map 0xFEBB1000 (virtual) to 0xFEBB1000 (physical)



    ; Step 3: Map P1 entry for 0xFEBB9000 to the physical address
    mov eax, 0xFEBB9000        ; Physical address of the AHCI BAR5
    or eax, 0b11               ; Present + Writable
    mov [p1_table + 8*(0x1B9)], eax  ; Map 0xFEBB9000 (virtual) to 0xFEBB9000 (physical)


    ; Step 3: Map P1 entry for 0xFEBBB000 to the physical address
    mov eax, 0xFEBBB000        ; Physical address of the AHCI BAR5
    or eax, 0b11               ; Present + Writable
    mov [p1_table + 8*(0x1BB)], eax  ; Map 0xFEBBB000 (virtual) to 0xFEBBB000 (physical)

    

    ; ENABLE PAGING
    ; load P4 to cr3 register (cpu uses this to access the P4 table)
    mov eax, p4_table
    mov cr3, eax

    ; enable PAE-flag in cr4 (Physical Address Extension)
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; set the long mode bit in the EFER MSR (model specific register)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging in the cr0 register
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ; Load GDT and jump to 64 bit long mode
    lgdt [gdt64.pointer]
    jmp gdt64.code:long_mode_start

    hlt

    
.no_multiboot:
    mov al, "0"
    jmp error

.no_cpuid:
    mov al, "1"
    jmp error

   
.no_long_mode:
    mov al, "2"
    jmp error
    

; Prints `ERR: ` and the given error code to screen and hangs.
; parameter: error code (in ascii) in al
error:
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte  [0xb800a], al
    hlt

section .bss
align 4096
p4_table:
    resb 4096
p3_table:
    resb 4096
p2_table_0:
    resb 4096
p2_table_1:
    resb 4096
p2_table_2:
    resb 4096
p2_table_3:
    resb 4096
p2_table_4:
    resb 4096
p1_table:
    resb 4096
p1_table_2:
    resb 4096
stack_bottom:
    resb 64
stack_top:

section .rodata
gdt64:
    dq 0 ; zero entry
.code: equ $ - gdt64 ; new
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; code segment
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

global GDT_OFFSET_KERNEL_CODE
%define GDT_OFFSET_KERNEL_CODE gdt64.code
