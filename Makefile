NAME = ntibux


OBJS = boot.o kernel.o misc.o kmisc.o terminal.o terminal_printk.o vga.o interpreter.o gdt.o mem.o list.o debug.o idt.o interrupt_handlers.o interrupts.o timer.o scheduler.o asm_misc.o

I686_GCC_DIR = $(HOME)/opt/cross/bin/
PATH := $(I686_GCC_DIR):$(PATH)

# C/C++
CC = i686-elf-g++

CFLAGS := $(CFLAGS) -Wall -Wextra -ffreestanding -nostdlib -masm=intel
CPPFLAGS := $(CPPFLAGS) -fno-exceptions -fno-rtti -fno-builtin -fno-stack-protector -nodefaultlibs

# assembly
AS = i686-elf-as
ASFLAGS =

# emulation
EM = qemu-system-i386
EMFLAGS := -curses



all: $(NAME)


$(NAME): $(OBJS)
	$(CC) -T linker.ld -o $(NAME) $(CFLAGS) $(CPPFLAGS) $(OBJS) -lgcc
	grub-file --is-x86-multiboot $(NAME)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

%.o: %.s
	$(AS) $< -o $@ $(ASFLAGS)


iso: $(NAME).iso

$(NAME).iso: $(NAME) grub.cfg
	mkdir -p isodir/boot/grub
	cp $(NAME) isodir/boot/$(NAME)
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(NAME).iso isodir


krun: $(NAME) # run kernel
	$(EM) -kernel $(NAME) $(EMFLAGS)

dkrun: $(NAME) # run kernel in debug
	$(EM) -kernel $(NAME) $(EMFLAGS) -s -S

run: $(NAME).iso # run iso
	$(EM) -hda ntibux.iso $(EMFLAGS)

drun: $(NAME).iso # run iso in debug
	$(EM) -hda ntibux.iso $(EMFLAGS) -s -S

dre: CFLAGS += -DDEBUG -g3
dre: ASFLAGS += -g
dre: re


clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
