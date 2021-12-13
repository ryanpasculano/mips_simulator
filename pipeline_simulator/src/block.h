/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <cstdint>
#include <cassert>
#include "util.h"

/*
 * Cache Block
 */
class Block {
private:
	uint8_t blkSize;
	uint8_t* data;
	uint32_t tag;
	bool dirty;
	bool valid; 
	uint32_t lastUsed; //for LRU and and pseudo LRU

public:
	bool getValid() {
		return valid;
	}

	bool getDirty() {
		return dirty;
	}

	uint32_t getTag() {
		return tag;
	}

	uint8_t* getData() {
		return data;
	}

	void setValid(bool flag) {
		valid = flag;
	}

	void setDirty(bool flag) {
		dirty = flag;
	}

	void clear(uint32_t blk_tag) {
		dirty = false;
		tag = blk_tag;
		valid = true;
	}
	void updateLast(uint32_t num) {
		lastUsed = num;
	}

	uint32_t getLastUsed() {
		return lastUsed;
	}

	Block(uint8_t blkSize) :
			blkSize(blkSize) {
		valid = false;
		dirty = false;
		data = new uint8_t[blkSize];
	}

	~Block() {
		delete (data);
	}

};

#endif
