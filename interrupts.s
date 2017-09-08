# mdr
.intel_syntax noprefix

# make all interrupt handlers converge to one common handler calling extern c interrupt_handler
.macro isr_noerr nbr
    .global isr_\nbr
    isr_\nbr:
    cli
    push 0 # push dummy error code
    push \nbr
    jmp interrupt_common_handler
.endm

.macro isr_err nbr
    .global isr_\nbr
    isr_\nbr:
    cli
    push \nbr
    jmp interrupt_common_handler
.endm

.macro pic_isr nbr
    .global pic_isr_\nbr
    pic_isr_\nbr:
    cli
    push \nbr # err_code = real interrupt number
    push \nbr + 32 // irq [0-15] are mapped to [32-47]
    jmp pic_interrupt_common_handler
.endm
 
isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_err 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_err 30
isr_noerr 31

pic_isr 0
pic_isr 1
pic_isr 2
pic_isr 3
pic_isr 4
pic_isr 5
pic_isr 6
pic_isr 7
pic_isr 8
pic_isr 9
pic_isr 10
pic_isr 11
pic_isr 12
pic_isr 13
pic_isr 14
pic_isr 15

interrupt_common_handler:
    pushad # push eax, ecx, edx, ebx, esp, ebp, esi, edi

    mov ax, ds
    push eax

    mov ax, 0x10 // kernel data selector (gdt[2])
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call interrupt_handler

    pop eax // pop ds
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popad # pop edi, esi, ebp, esp, ebx, edx, ecx, eax

    add esp, 8 # error code, interrupt nbr

    sti
    iret

pic_interrupt_common_handler:
    pushad # push eax, ecx, edx, ebx, esp, ebp, esi, edi

    mov ax, ds
    push eax

    mov ax, 0x10 // kernel data selector (gdt[2])
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call pic_interrupt_handler

    pop eax // pop ds
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popad # pop edi, esi, ebp, esp, ebx, edx, ecx, eax

    add esp, 8 # error code, interrupt nbr

    sti
    iret
