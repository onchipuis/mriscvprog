.text;
.global _start;

#### BEGIN _start
_start:
	li s0, 0x1040;		# GPIO address
	add s1, zero, zero;	# GPIO status 1
	#ori s1, zero, 0x7;	# This is if you want strong driver
infinite:
	li t0, 0x6000;	# Counter
	li t1, 0x1;
	for_counter_1:
		sub t0, t0, t1;
		bne t0, zero, for_counter_1;
	# Sum thing...
	sw s1, 0(s0);		# Write to GPIO 1
	addi s0, s0, 4;
	li t2, 0x1058;
	bne s0, t2, infinite;
	li s0, 0x1040;		# GPIO address
	xori s1, s1, 0x3;	# Toogle GPIO status
	jal zero, infinite;
	

#riscv32-unknown-elf-gcc -c test5.S
#riscv32-unknown-elf-objdump -d test5.o 
#riscv32-unknown-elf-gcc -Os -m32 -ffreestanding -nostdlib -o test5.elf -Wl,-Bstatic,-T,sections.lds,-Map,test5.map,--strip-debug test5.o -lgcc
#riscv32-unknown-elf-objcopy -O binary test5.elf test5.dat

