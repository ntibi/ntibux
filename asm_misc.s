.intel_syntax noprefix

.section .text


.global copy_page
copy_page:

    cli

    mov edx, cr0
    and edx, 0x7fffffff # paging bit off
    mov cr0, edx

    cld
    mov ecx, 4096 / 4 # PAGESIZE / sizeof(dword)
    rep movsd

    mov edx, cr0
    or edx, 0x80000000 # paging bit on
    mov cr0, edx

    sti

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
    sti
    ret
    1:
    ret
