nasm -f elf a.asm && ld -s -Ttext 0x400000 -o a.bin a.o
nasm -f elf b.asm && ld -s -Ttext 0x402000 -o b.bin b.o
nasm -f elf c.asm && ld -s -Ttext 0x404000 -o c.bin c.o
nasm -f elf d.asm && ld -s -Ttext 0x406000 -o d.bin d.o
