/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __MAP_TABLE__
#define __MAP_TABLE__
#include <cstdint>
#define NUM_NAMES 32
 /*
  * Map Table structure
  * Should be used for both the "Map Table" 
  * and the "Architectural Table"
  */

typedef struct mapTableStruct {
	uint32_t archReg;
	uint32_t pReg;
	bool ready;
};


class MapTable {
public:
	mapTableStruct table[NUM_NAMES];

	MapTable();
	~MapTable();
	
	void addMap(uint32_t archReg, uint32_t pReg);
	void setReady(uint32_t archReg);
	bool getReady(uint32_t archReg);
	int getPRegId(uint32_t archReg);
	void swapPReg(uint32_t, uint32_t);
};

#endif
