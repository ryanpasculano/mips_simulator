/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */
 /*
  * Reservation Station structure
  */

#ifndef __BRANCH_STACK__
#define __BRANCH_STACK__
#include <cstdint>
#include <util.h>
#include "rs.h"
#include "rob.h"
#include "map_table.h"
#include "freelist.h"
#include "lsq.h"
#define MAX_BRANCHES 4

class BranchStack {
public:
	BranchStack();
	~BranchStack();

	uint32_t branchMask;
	MapTable* mapTables[MAX_BRANCHES];
	ROB* robs[MAX_BRANCHES];
	RS* rss[MAX_BRANCHES];
	FreeList* freelists[MAX_BRANCHES];
	LSQ* lsqs[MAX_BRANCHES];


	void updateMask(uint32_t branch_mask);
	uint32_t getMask();
	void addSet(uint32_t mask, MapTable*, ROB*, RS*, FreeList*, LSQ*);
};

#endif
