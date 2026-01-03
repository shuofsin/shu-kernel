// kernel.c

#include "keyboard_map.h"

#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08

#define ENTER_KEY_CODE 0x1C

#define COLOR 0x09

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
extern void disable_cursor(void);
extern void enable_cursor(void);
extern void set_cursor(unsigned short x, unsigned short y);

unsigned int current_loc = 0;
char *vidptr = (char*)0xB8000;

void idt_init(void);
void kb_init(void);
void kprint(const char *str);
void kprint_newline(void);
void clear_screen(void);
void keyboard_handler_main(void);
void update_cursor(void);

// run the kernel
void kmain(void) {
	const char *str = "shu-kernel, now with keyboard support!";
	clear_screen();
	kprint(str);
	kprint_newline();
	kprint_newline();

	idt_init();
	kb_init();

	while(1);
}


// setup interrupt descriptor table
struct IDT_entry {
	unsigned short int offset_lowerbits;
	unsigned short int selector;
	unsigned char zero;
	unsigned char type_attr;
	unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

void idt_init(void) {
	unsigned long keyboard_address;
	unsigned long idt_address;
	unsigned long idt_ptr[2];

	// create keyboard interrupt entry
	keyboard_address = (unsigned long)keyboard_handler;
	IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
	IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
	IDT[0x21].zero = 0;
	IDT[0x21].type_attr = INTERRUPT_GATE;
	IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

	// ICW1 begin init
	write_port(0x20, 0x11);
	write_port(0xA0, 0x11);


	// ICW2 remap offset
	// in x86, the first 32 interrupts are reserved
	write_port(0x21, 0x20);
	write_port(0xA1, 0x28);

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

void kb_init(void) {
	// 0xFD enables only IRQ1 (keyboard)
	write_port(0x21, 0xFD);
}

void kprint(const char *str) {
	unsigned int i = 0;
	while (str[i] != '\0') {
		vidptr[current_loc++] = str[i++];
		vidptr[current_loc++] = 0x09;
	}
	update_cursor();
}

void kprint_newline(void) {
	unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
	current_loc = current_loc + (line_size - current_loc % (line_size));
	update_cursor();
}

void clear_screen(void) {
	unsigned int i = 0;
	while (i < SCREENSIZE) {
		vidptr[i++] = ' ';
		vidptr[i++] = 0x09;
	}
}

void keyboard_handler_main(void) {
	unsigned char status;
	char keycode;

	// write EOI
	write_port(0x20, 0x20);

	status = read_port(KEYBOARD_STATUS_PORT);
	
	// Lowest bit of status will be set if buffer is not empty
	if (status & 0x01) {
		keycode = read_port(KEYBOARD_DATA_PORT);
		if (keycode < 0)
			return;

		if (keycode == ENTER_KEY_CODE) {
			kprint_newline();
			return;
		}

		vidptr[current_loc++] = keyboard_map[(unsigned char) keycode];
		vidptr[current_loc++] = 0x09;
		update_cursor();
	}
}

void update_cursor(void) {
	set_cursor((unsigned short)((current_loc / 2) % 80), (unsigned short)((current_loc / 2) / 80));
}









