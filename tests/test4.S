.text;
.global _start;

#### BEGIN _start
_start:
	li s0, 0x1040;							# GPIO address
	li s1, 0x1;								# GPIO status
	li s2, 0x0;								# GPIO direction
infinite:
	# Explore all the GPIO and write
	li t0, 0x6;								# t0 = 6;						
	li t1, 0x1;
	for_counter_2:
		sub t0, t0, t1;						# t0--;
		# Calculation of address
		slli t2, t0, 2;						# t2 = t0 * 4;
		add t3, s0, t2; 					# t3 = 0x1040 + (t0 * 4); // t3 will be the address for t0'th GPIO
		# Calculation of data
		srl t2, s1, t0; 					# t2 = s1 >> t0;
		andi t2, t2, 0x1; 					# t2 = (s1 >> t0) & 0x1;
		add t4, zero, zero;
		beq t2, zero, if_assign_zero;		# if(t2 == zero) t4 = 0x0;
			li t4, 0x7;						# else t4 = 0x7;
		if_assign_zero:
		# Final assign to GPIO	
 		sw t4, 0(t3);						# Write to GPIO t3, t4 data
		bne t0, zero, for_counter_2;  		# for(t0 = 6; t0 != 0; t0--)
	# Counter for delay
	li t0, 0x6000;							
	li t1, 0x1;
	for_counter_1:
		sub t0, t0, t1;
		bne t0, zero, for_counter_1;
	# Logic thing to do the spectacular car
	beq s2, zero, if_s2_eq_zero;			# if(s2 != 0) s1 = s1 >> 1;
		srli s1, s1, 1;
		jal zero, if_s2_neq_zero;
	if_s2_eq_zero:							# if(s2 == 0) s1 = s1 << 1;
		slli s1, s1, 1;
	if_s2_neq_zero:							# endif
	bne s1, zero, if_s1_neq_zero_secure;
		li s1, 0x1;
	if_s1_neq_zero_secure:
	# Logic for change direction of spectacular car
	li t0, 0x20;
	li t1, 0x1;
	bne s1, t0, if_s1_eq_t0;				# if(s1 == 0x20) s2 = 1;
		li s2, 0x1;
		jal zero, if_s1_neq_t0_t1;
	if_s1_eq_t0:
		bne s1, t1, if_s1_neq_t0_t1;		# else if(s1 == 0x1) s2 = 0;
			li s2, 0x0;
	if_s1_neq_t0_t1:						# endif
	jal zero, infinite;
	

#riscv32-unknown-elf-gcc -c test4.S
#riscv32-unknown-elf-objdump -d test4.o 
#riscv32-unknown-elf-gcc -Os -m32 -ffreestanding -nostdlib -o test4.elf -Wl,-Bstatic,-T,sections.lds,-Map,test4.map,--strip-debug test4.o -lgcc
#riscv32-unknown-elf-objcopy -O binary test4.elf test4.dat

