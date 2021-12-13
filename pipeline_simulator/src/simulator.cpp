/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdio>
#include <iostream>
#include "simulator.h"
#include "util.h"

uint64_t currCycle;

Simulator::Simulator(MemHrchyInfo* info) {
	currCycle = 0;
	printf("initialize simulator\n\n");
	//initializing core
	pipe = new PipeState(info-> ras_size, info -> bht_entries, info -> bht_entry_width, info -> pht_width);

	/*
	 * initializing memory hierarchy
	 *  you should update this for adding the caches
	 */
	main_memory = new BaseMemory(info->memDelay);
	l2_cache = new Cache(info->cache_size_l2, info->cache_assoc_l2, info->cache_blk_size,
			(ReplacementPolicy) info->repl_policy_l2, info->access_delay_l2, 
			info->cache_l2_mshr_entries, info->cache_l2_mshr_subentries, 
			info->cache_l2_wbb_entries, info->writeThrough, info->writeBack, "L2");	
	l1i_cache = new Cache(info->cache_size_l1, info->cache_assoc_l1, info->cache_blk_size,
			(ReplacementPolicy)info->repl_policy_l1i, info->access_delay_l1,
			info->cache_l1_mshr_entries, info->cache_l1_mshr_subentries, 
			info->cache_l1_wbb_entries, info->writeThrough, info->writeBack, "L1I");
	l1d_cache = new Cache(info->cache_size_l1, info->cache_assoc_l1, info->cache_blk_size,
			(ReplacementPolicy)info->repl_policy_l1d, info->access_delay_l1,
			info->cache_l1_mshr_entries, info->cache_l1_mshr_subentries, 
			info->cache_l1_wbb_entries, info->writeThrough, info->writeBack, "L1D");

	//set next for each memory
	main_memory->next = nullptr;
	l2_cache->next = main_memory;
	l1i_cache->next = l2_cache;
	l1d_cache->next = l2_cache;

	//set prev for each memory
	main_memory->prev = l2_cache;
	l2_cache->prev = l1d_cache;
	l1i_cache->prev = pipe;
	l1d_cache->prev = pipe;
	
	// set prev_instruction
	main_memory->prev_instruction = l2_cache;
	l2_cache->prev_instruction = l1i_cache;
	l1i_cache->prev_instruction = pipe;
	l1d_cache->prev_instruction = pipe;

	//set the first memory in the memory-hierarchy
	pipe->data_mem = l1d_cache;
	pipe->inst_mem = l1i_cache;
}



void Simulator::cycle() {
	//check if memory needs to respond to any packet in this cycle
	main_memory->Tick();
	l2_cache->Tick();
	l1d_cache->Tick();
	l1i_cache->Tick();
	//progress of the pipeline in this clock
	pipe->pipeCycle();
	pipe->stat_cycles++;
	// increment the global clock of the simulator
	currCycle++;
}



void Simulator::run(int num_cycles) {
	int i;
	if (pipe->RUN_BIT == false) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating for %d cycles...\n\n", num_cycles);
	for (i = 0; i < num_cycles; i++) {
		if (pipe->RUN_BIT == false) {
			printf("Simulator halted\n\n");
			break;
		}
		cycle();
	}
}


void Simulator::go() {
	if (pipe->RUN_BIT == false) {
		printf("Can't simulate, Simulator is halted\n\n");
		return;
	}

	printf("Simulating...\n\n");
	while (pipe->RUN_BIT)
		cycle();
	printf("Simulator halted\n\n");
}



uint32_t Simulator::readMemForDump(uint32_t address) {
	uint32_t data = NULL;
	//you should update this since adding the caches
	pipe->data_mem->dumpRead(address, 4, (uint8_t*) &data);
	if (data == NULL) {
		l2_cache -> dumpRead(address, 4, (uint8_t*) &data); 
	}
	if (data == NULL){
		main_memory -> dumpRead(address, 4, (uint8_t*) &data);
	}
	return data;
}

void Simulator::registerDump() {
	int i;

	printf("PC: 0x%08x\n", pipe->PC);

	for (i = 0; i < 32; i++) {
		printf("R%d: 0x%08x\n", i, pipe->REGS[i]);
	}

	printf("HI: 0x%08x\n", pipe->HI);
	printf("LO: 0x%08x\n", pipe->LO);
	printf("Cycles: %u\n", pipe->stat_cycles);
	printf("FetchedInstr: %u\n", pipe->stat_inst_fetch);
	printf("RetiredInstr: %u\n", pipe->stat_inst_retire);
	printf("IPC: %0.3f\n",
			((float) pipe->stat_inst_retire) / pipe->stat_cycles);
	printf("Flushes: %u\n", pipe->stat_squash);
}

void Simulator::memDump(int start, int stop) {
	int address;

	printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------\n");
	for (address = start; address < stop; address += 4) {
		printf("MEM[0x%08x]: 0x%08x\n", address, readMemForDump(address));
	}
	printf("\n");
}

Simulator::~Simulator() {
	delete main_memory;
	delete l2_cache;
	delete l1d_cache;
	delete l1i_cache;
	delete pipe;
}
