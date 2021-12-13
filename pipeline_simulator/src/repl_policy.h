/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __REPL_POLICY_H__
#define __REPL_POLICY_H__

#include"block.h"

class Cache;

/*
 * AbstarctReplacementPolicy should be inherited by your
 * implemented replacement policy
 */
class AbstarctReplacementPolicy {
public:
	AbstarctReplacementPolicy(Cache* cache);
	virtual ~AbstarctReplacementPolicy() {}
	//pointer to the Cache
	Cache* cache;
	/*
	 * should return the victim block based on the replacement
	 * policy- the caller should invalidate this block
	 */
	virtual int getVictim(uint32_t addr, bool isWrite) = 0;
	/*
	 * Called for both hit and miss.
	 * Should update the replacement policy metadata.
	 */
	virtual void update(uint32_t addr, int way, bool isWrite) = 0;
};

/*
 * Random replacement policy
 */
class RandomRepl: public AbstarctReplacementPolicy {
public:
	RandomRepl(Cache* cache);
	~RandomRepl() {}
	virtual int getVictim(uint32_t addr, bool isWrite) override;
	virtual void update(uint32_t addr, int way, bool isWrite) override;
};

/*
 * LRU replacement policy
 */
class LRURepl : public AbstarctReplacementPolicy {
public:
	LRURepl(Cache* cache);
	~LRURepl() {}
	virtual int getVictim(uint32_t addr, bool isWrite) override;
	virtual void update(uint32_t addr, int way, bool isWrite) override;
};

/*
 * Pseudo-LRU replacement policy
 */
class PLRURepl : public AbstarctReplacementPolicy {
public:
	PLRURepl(Cache* cache);
	~PLRURepl() {}
	virtual int getVictim(uint32_t addr, bool isWrite) override;
	virtual void update(uint32_t addr, int way, bool isWrite) override;
};

#endif
