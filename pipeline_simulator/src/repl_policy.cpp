/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include<set>
#include "repl_policy.h"
#include "cache.h"

using namespace std;

AbstarctReplacementPolicy::AbstarctReplacementPolicy(Cache* cache) :
		cache(cache) {
}

RandomRepl::RandomRepl(Cache* cache) :
		AbstarctReplacementPolicy(cache) {
}

int RandomRepl::getVictim(uint32_t addr, bool isWrite) {
	addr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & addr;

	//first check if there is a free block to allocate
	for (int i = 0; i < (int) cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getValid() == false) {
			//return cache->blocks[setIndex][i];
			return i;
		}
	}
	//randomly choose a block
	int victim_index = rand() % cache->getAssociativity();
	//return cache->blocks[setIndex][victim_index];
	return victim_index;
}

void RandomRepl::update(uint32_t addr, int way, bool isWrite) {
	//there is no metadata to update:D
	return;
}


LRURepl::LRURepl(Cache* cache) :
	AbstarctReplacementPolicy(cache) {
}

int LRURepl::getVictim(uint32_t addr, bool isWrite) {
	addr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & addr;

	//first check if there is a free block to allocate
	for (int i = 0; i < (int)cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getValid() == false) {
			//return cache->blocks[setIndex][i];
			return i;
		}
	}
	//get the block with the lowest lastUsed value
	int victim_index = rand() % cache->getAssociativity();
	for (int i = 0; i < (int)cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getLastUsed() < victim_index) {
			victim_index = cache->blocks[setIndex][i]->getLastUsed();
		}
	}
	//return cache->blocks[setIndex][victim_index];
	return victim_index;
}

void LRURepl::update(uint32_t addr, int way, bool isWrite) {
	//lastUsed updating handled in cache.cpp

}

PLRURepl::PLRURepl(Cache * cache) :
	AbstarctReplacementPolicy(cache) {
}

int PLRURepl::getVictim(uint32_t addr, bool isWrite) {
	addr = addr / cache->getBlockSize();
	uint64_t setIndex = (cache->getNumSets() - 1) & addr;
	set<int> st;
	set<int>::iterator it = st.begin();
	int arr[4] = { 0,1,2,3 };
	st.insert(arr, arr + 4);

	//first check if there is a free block to allocate
	for (int i = 0; i < (int)cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getValid() == false) {
			//return cache->blocks[setIndex][i];
			return i;
		}
	}
	//get block with max lastUsed 
	int max = 0;
	for (int i = 0; i < (int)cache->getAssociativity(); i++) {
		if (cache->blocks[setIndex][i]->getLastUsed() > max) {
			max = cache->blocks[setIndex][i]->getLastUsed();
		}
	}
	//select any block just not the one with max lastUsed
	st.erase(max);
	int k = rand() % ((int)cache->getAssociativity() - 1);
	//get that index value from the set
	for (int i = 0; i < k; i++) {
		it++;
	}
	int victim_index = *it;
	//return cache->blocks[setIndex][victim_index];
	return victim_index;

}

void PLRURepl::update(uint32_t addr, int way, bool isWrite) {
	//lastUsed update hanled in cache.cpp
}
