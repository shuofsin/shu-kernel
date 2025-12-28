;; kernel.asm

bits 32

section .text
	; multiboot spec
	align 4
	dd 0x1BADB002		; magic number
	dd 0x00			; flags
	dd - (0x1BADB002 + 0x00); checksum (m + f + c = 0)


global start
extern kmain			; defined in kernel.c

start:
	cli			; block interrupts
	mov esp, stack_space	; set stack pointer
	call kmain		
	hlt			; halt the cpu

section .bss
resb 8192			; 8KB for stack pointer
stack_space:
