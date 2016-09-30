.text;
.global _start;

#### BEGIN _start
_start:
	li s0, 0x1040;		# GPIO address
	add s1, zero, zero;	# GPIO status
	#ori s1, zero, 0x7;	# This is if you want strong driver
infinite:
	li t0, 0x100000;	# Counter
	li t1, 0x1;
	for_counter_1:
		sub t0, t0, t1;
		bne t0, zero, for_counter_1;
	xori s1, s1, 0x7;	# Toogle GPIO status
	andi s1, s1, 0x7;
	sw s1, 0(s0);		# Write to GPIO 1
	sw s1, 4(s0);		# Write to GPIO 2
	sw s1, 8(s0);		# Write to GPIO 3
	sw s1, 12(s0);		# Write to GPIO 4
	sw s1, 16(s0);		# Write to GPIO 5
	sw s1, 20(s0);		# Write to GPIO 6
	#sw s1, 24(s0);		# Write to GPIO 7
	#sw s1, 28(s0);		# Write to GPIO 8
	jal zero, infinite;
	

#riscv32-unknown-elf-gcc -c test1.S
#riscv32-unknown-elf-objdump -d test1.o 
#riscv32-unknown-elf-gcc -Os -m32 -ffreestanding -nostdlib -o test1.elf -Wl,-Bstatic,-T,sections.lds,-Map,test1.map,--strip-debug test1.o -lgcc
#riscv32-unknown-elf-objcopy -O binary test1.elf test1.dat

