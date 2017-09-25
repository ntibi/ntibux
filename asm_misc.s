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

.global eflags
eflags:
    pushf
    pop eax
    ret


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
