// kernel.c

#include "keyboard_map.h"


extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);

unsigned int current_loc = 0;

// run the kernel
void kmain(void) {

	const char *str = "shu-kernel";
	char *vidptr = (char*)0xB8000; // video memory begins here
	unsigned int i = 0;
	unsigned int j = 0;

	// Clear the screen
	while (j < 80 * 25 * 2) {
		vidptr[j] = ' ';
		vidptr[j+1] = 0x0C;
		j = j + 2;
	}

	j = 0;

	// Write string to screen
	while (str[j] != '\0') {
		vidptr[i] = str[j];
		vidptr[i+1] = 0x0C;
		++j;
		i = i + 2;
	}
	return;
}

// setup interrupt descriptor table

struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
}

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void) {
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];


	// create keyboard interrupt entry
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = 0x08; // kernel code offset
	IDT[0x21].zero = 0;
	IDT[0x31].type_attr = 0x8e; // interrupt gate
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	// ICW1 begin init
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);


	// ICW2 remap offset
	// in x86, the first 32 interrupts are reserved
	write_port(0x21, 0x20);
	write_port(0x21, 0x28);

	// ICW3 setup cascading
	write_port(0x21, 0x00);
	write_port(0xA1, 0x00);

	// ICW4 environment info
	write_port(0x21, 0x01);
	write_port(0xA1, 0x01);

	// mask interrupts
	write_port(0x21, 0xff);
	write_port(0xA1, 0xff);

	// fill the IDT descriptor
	idt_address = (unsigned long)IDT;
	idt_ptr[0] = (sizeof (struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
	idt_ptr[1] = idt_address >> 16;

	load_idt(idt_ptr);
}

















