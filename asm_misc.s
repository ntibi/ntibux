.intel_syntax noprefix

.section .text


.global copy_page
copy_page:

    call pop_ints

    mov edx, cr0
    and edx, 0x7fffffff # paging bit off
    mov cr0, edx

    cld
    mov ecx, 4096 / 4 # PAGESIZE / sizeof(dword)
    rep movsd

    mov edx, cr0
    or edx, 0x80000000 # paging bit on
    mov cr0, edx

    call push_ints

    ret

.global context_switch # __fastcall (     ecx,     edx)
context_switch: #                   (*old_esp, new_esp)

    lea eax, 1f
    push eax

    pushf
    pushad

    mov [ecx], esp
    mov esp, edx

    popad
    popf
    call push_ints
    ret
    1:
    ret

.global get_eflags
get_eflags:
    pushf
    pop eax
    ret

.global get_eip
get_eip:
    mov eax, [esp]
    ret

.global get_eax
get_eax:
    ret

.macro get_reg r
    .global get_\r
    get_\r:
    mov eax, \r
    ret
.endm

get_reg ebx
get_reg ecx
get_reg edx
get_reg edi
get_reg esi
get_reg ebp
get_reg esp
get_reg cr0
get_reg cr2
get_reg cr3
get_reg cr4


.extern interrupts_semaphore

.global push_ints # TODO: improve these two funcs
push_ints:
    cli
    push eax
    mov eax, [interrupts_semaphore]
    inc eax
    mov [interrupts_semaphore], eax
    cmp eax, 1
    pop eax
    jl 1f
    sti
    1:
    ret

.global pop_ints
pop_ints:
    cli
    push eax
    mov eax, [interrupts_semaphore]
    dec eax
    mov [interrupts_semaphore], eax
    cmp eax, 1
    pop eax
    jl 1f
    sti
    1:
    ret

.global busy_loop
busy_loop:
    hlt
    jmp busy_loop
