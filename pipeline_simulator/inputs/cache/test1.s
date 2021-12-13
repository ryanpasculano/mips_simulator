.text					
    lui $s0, 0x1000 # data memory: 0x10000000						PC 400000

    lui $t0, 1 # loop count: 0x10000							PC 400004
Loop:
    sll $t1, $t0, 2					#				PC 400008
    addu $t1, $t1, $s0					#				PC 40000c
    sw $t0, 0($t1) # cache miss every 8 iterations					PC 400010
    sw $t0, 4($t1) # never a cache miss: trails first store in descending stream	PC 400014
    addiu $t0, $t0, -1
    bne $t0, $0, Loop

    addiu $v0, $0, 10
    syscall
