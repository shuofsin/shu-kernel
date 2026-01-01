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
global disable_cursor
global enable_cursor
global update_cursor

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
	ret

keyboard_handler:
	call 	keyboard_handler_main
	iretd

start:
	cli			; blocks interrupts
	mov esp, stack_space
	call kmain
	hlt 			; halt the cpu

disable_cursor:
	pushf
	push eax
	push edx

	mov dx, 0x3D4
	mov al, 0xA 		; low cursor shape register
	out dx, al

	inc dx
	mov al, 0x20		; bits 6-7 unused, bit 5 disables cursor, bits 0-4 control the cursor shape
	out dx, al

	pop edx
	pop eax
	popf
	ret

enable_cursor:
	pushf
	push eax
	push edx
	
	mov dx, 0x3D4
	mov al, 0xA		; low cursor shape register
	out dx, al

	inc dx
	mov al, 0x0D		; bits 6-7 unused, bit 5 disables cursor, bits 0-4 control the cursor shape
	out dx, al
	
	pop edx
	pop eax
	popf
	ret

update_cursor: 
VGA.Width equ 80

.SetCoords:
	mov dl, VGA.Width
	mul dl
	add bx, ax

.SetOffset:
	mov dx, 0x03D4
	mov al, 0x0F
	out dx, al
	
	inc dl
	mov al, bl
	out dx, al
	
	dec dl
	mov al, 0x0E
	out dx, al
	
	inc dl
	mov al, bh
	out dx, al
	ret
	
section .bss
resb 8192			; 8KB for stack pointer
stack_space:
