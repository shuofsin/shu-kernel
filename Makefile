kernel-001: kc.o
	ld -m elf_i386 -T link.ld -o kernel-001 kasm.o kc.o

kc.o: kasm.o
	gcc -fno-stack-protector -m32 -c kernel.c -o kc.o

kasm.o:
	nasm -f elf32 kernel.asm -o kasm.o

clean: 
	rm kasm.o kc.o kernel-001
