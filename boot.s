# Declare constants for the multiboot header.
.intel_syntax noprefix
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS) # CHECKSUM + MAGIC + FLAGS = 0


.section .multiboot # multiboot header
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM


.section .bss

.align 16
.global stack_bottom
.global stack_top

stack_bottom:
.skip 16384 # 16 KB for bss + stack
stack_top:


.section .text
.global _start
.type _start, @function
_start:
    lea ebp, stack_top
    lea esp, stack_top
    push eax # multiboot header magic
    push ebx # multiboot header pointer
    call kernel_main
    cli
_hlt:
    hlt
    jmp _hlt


.size _start, . - _start
