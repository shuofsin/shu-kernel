// kernel.c

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
