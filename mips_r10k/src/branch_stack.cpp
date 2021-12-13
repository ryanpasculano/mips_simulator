#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include "util.h"
#include "branch_stack.h"

BranchStack::BranchStack()
{
	branchMask = 0;
	for (int i = 0; i < MAX_BRANCHES; i++) {
		mapTables[i] = nullptr;
		robs[i] = nullptr;
		rss[i] = nullptr;
		freelists[i] = nullptr;
		lsqs[i] = nullptr;
	}
}

BranchStack::~BranchStack()
{
	//nothing to do
}

void BranchStack::updateMask(uint32_t branch_mask)
{
	branchMask = branch_mask;
}

uint32_t BranchStack::getMask()
{
	return branchMask;
}

void BranchStack::addSet(uint32_t mask, MapTable* mapTable, ROB* rob, RS* rs, FreeList* freelist, LSQ* lsq)
{
	branchMask = mask;
	mapTables[branchMask] = mapTable;
	robs[branchMask] = rob;
	rss[branchMask] = rs;
	freelists[branchMask] = freelist;
	lsqs[branchMask] = lsq;
}

