/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __BASE_MEMORY_H__
#define __BASE_MEMORY_H__
#include <cstdint>
#include <cstdlib>
#include <assert.h>
#include "abstract_memory.h"

//initializing memory regions
#define MEM_DATA_START  0x10000000
#define MEM_DATA_SIZE   0x00100000
#define MEM_TEXT_START  0x00400000
#define MEM_TEXT_SIZE   0x00100000
#define MEM_STACK_START 0x7ff00000
#define MEM_STACK_SIZE  0x00100000
#define MEM_KDATA_START 0x90000000
#define MEM_KDATA_SIZE  0x00100000
#define MEM_KTEXT_START 0x80000000
#define MEM_KTEXT_SIZE  0x00100000
#define MEM_NREGIONS 5

typedef struct {
	uint32_t start, size;
	uint8_t *mem;
} mem_region_t;


/*
 * Main memory
 */
class BaseMemory: public AbstractMemory {
public:
	BaseMemory();
	virtual ~BaseMemory();
	virtual bool sendReq(Packet * pkt) override;
	virtual void recvResp(Packet* readRespPkt) override;
	virtual void Tick() override;
	void dumpRead(uint32_t addr, uint32_t size, uint8_t* data);
	/*
	 * for the given address and size will return the
	 * corresponding memory region
	 */
	mem_region_t* getMemRegion(uint32_t addr, uint32_t size);

	mem_region_t MEM_REGIONS[MEM_NREGIONS];
};

#endif
