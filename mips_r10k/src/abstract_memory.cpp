/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "abstract_memory.h"

AbstractMemory::AbstractMemory(uint32_t reqQueueCapacity) :
		reqQueueCapacity(reqQueueCapacity) {

}

AbstractMemory::~AbstractMemory() {
	// TODO Auto-generated destructor stub
}

