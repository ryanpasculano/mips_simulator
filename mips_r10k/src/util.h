/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __UTIL_H__
#define __UTIL_H__
#include <cstdint>

uint64_t extern currCycle;

extern bool DEBUG_MEMORY;
extern bool DEBUG_PIPE;
extern bool DEBUG_FETCH;
extern bool DEBUG_DISPATCH;
extern bool DEBUG_ISSUE;
extern bool DEBUG_EXECUTE;
extern bool DEBUG_COMPLETE;
extern bool DEBUG_RETIRE; 

extern bool TRACE_MEMORY;

#define DPRINTF(flag, fmt, ...) \
	if(flag) \
        fprintf(stdout, "Cycle %9lu : [%s][%s]%d: " fmt, currCycle, __FILE__, __func__, __LINE__, ##__VA_ARGS__);

#define TRACE(flag, cond, fmt, ...) \
	if((flag) && (cond)) \
        fprintf(stdout, fmt, ##__VA_ARGS__);


enum PacketSrcType {
	PacketTypeFetch = 0,
	PacketTypeLoad = 1,
	PacketTypeStore = 2,
	PacketTypePrefetch = 3
};

enum FUtype {
	ALU1 = 0,
	ALU2 =1,
	LD = 2,
	ST = 3,
	FP1 = 4,
	FP2 = 5
};

typedef struct tagStruct {
	bool ready;
	uint32_t tag;
};

class MemHrchyInfo{
public:
	
	uint64_t rob_size;
	uint32_t lsq_size;

	MemHrchyInfo() {
		rob_size = 8;
		lsq_size = 4;
	}
};

#endif
