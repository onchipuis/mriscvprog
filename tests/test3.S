.text;
.global _start;

#### BEGIN _start
_start:
	li s0, 0x1040;		# GPIO address
	add s1, zero, zero;	# GPIO status
infinite:
	add s1, zero, zero;
	sw s1, 0(s0);		# Write to GPIO 1
	sw s1, 4(s0);		# Write to GPIO 2
	sw s1, 8(s0);		# Write to GPIO 3
	sw s1, 12(s0);		# Write to GPIO 4
	sw s1, 16(s0);		# Write to GPIO 5
	sw s1, 20(s0);		# Write to GPIO 6
	#sw s1, 24(s0);		# Write to GPIO 7
	#sw s1, 28(s0);		# Write to GPIO 8
	jal zero, infinite;
	

#riscv32-unknown-elf-gcc -c test3.S
#riscv32-unknown-elf-objdump -d test3.o 
#riscv32-unknown-elf-gcc -Os -m32 -ffreestanding -nostdlib -o test3.elf -Wl,-Bstatic,-T,sections.lds,-Map,test3.map,--strip-debug test3.o -lgcc
#riscv32-unknown-elf-objcopy -O binary test3.elf test3.dat

