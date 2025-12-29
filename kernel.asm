;; kernel.asm

bits 32

section .text
	; multiboot spec
	align 4
	dd 0x1BADB002		; magic number
	dd 0x00			; flags
	dd - (0x1BADB002 + 0x00); checksum (m + f + c = 0)


global start
global keyboard_handler
global read_port
global write_port
global load_idt

extern kmain			; defined in kernel.c
extern keyboard_handler_main

read_port:
	mov edx, [esp + 4]
	in al, dx
	ret

write_port:
	mov edx, [esp + 4]
	mov al, [esp + 4 + 4]
	out dx, al
	ret

load_idt:
	mov edx, [esp + 4]
	lidt [edx]
	sti			; turn on interrupts

keyboard_handler:
	call keyboard_handler_main
	iretd

start:
	cli			; blocks interrupts
	mov esp, stack_space
	call main
	hlt 			; halt the cpu

section .bss
resb 8192			; 8KB for stack pointer
stack_space:
