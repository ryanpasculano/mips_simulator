# Project2

1. initialize map table with the first set of free registers always...never init empty map table

2. convert raw insts. to renamed insts. (in dispatch phase) with new physical register names
each instruction can be renamed individually (always rename the dest reg to one from the free list)

3. while renaming, don't overwrite the already used register....m
get a free register from the free list and map it and then use it to store the new value

4. when an inst. retires..add the physical register (TOLD) that was previously mapped to the
same logical register it was writing to into the free list.

5. for store inst, commit the store only when that particular inst has been retired. 

6. remove the WB stage, modify pipeline stages to be - Fetch, Dispatch, Issue, Execute, Complete, Retire

	The overview of all stages are as follows:

		Fetch: The pipeline fetches and aligns the next four instructions. (You can assume that all instructions 
			are 4bytes and aligned). Usually, the simulator decodes all four instructions during the next cycle,
			unless it encounters structural hazards. 
 

		Dispatch: Allocate RS(Reservation Station), ROB(Re-Order Buffer), LSQ(Load-Store Queue) entries and 
			new physical register and also record previously mapped physical register for the four instructions.
			The pipeline decodes and maps four instructions in parallel during this stage. The pipeline reads 
			preg (physical register) tags for input registers and store them in RS. Also, it reads preg tag 
			for output register(preg previously mapped to the logical output) and stores that in ROB. Moreover,
			it allocates new preg (from free list) for output register and stores that in RS, ROB, Map Table.
			Note the exceptions for integer multiply and divide instructions (Refer to the paper). In case of 
			structural hazard (RS, ROB, LSQ, physical registers) the pipeline stalls. Also in this stage, pipeline 
			calculates target addresses for jump and branch instructions. 
 

		Issue:  RS reads the ready-bit in the Map Table to determine if the operands are ready or not. 
			Instructions wait in the RS until all their operands are ready. The pipeline reads operands from 
			the register files during the second half of this stage, and execution begins in the next stage.
 

		Execute: The pipeline performs the ALU operation or data memory access in this stage and frees the
			entry in RS for this instruction. When loads execute, the pipeline accesses LSQ and memory in parallel
			(it forwards from LSQ if older store with matching address is in LSQ)
 

		Complete: In this stage, pipeline writes the destination physical register and sets the output’s 
			register ready bit in the map table. Also it sets ready bits for matching input tags in RS. 
			Completed store is written to LSQ.
 

		Retire: In this stage if the ROB head is not complete the pipeline stalls. For “Store” the pipeline 
			writes LSQ head to memory. Also it frees ROB, LSQ entries and previous physical register that it 
			records in the dispatch stage (returns it to head of free list). Also, it keeps tracks of committed 
			physical register (records in architectural map table).


7. 