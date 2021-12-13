/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __ABSTRACT_PREFETCHER_H__
#define __ABSTRACT_PREFETCHER_H__

#include <cstdint>

class Cache;

class AbstractPrefetcher {
public:
	//pointer to the cache
	Cache* cache;
	AbstractPrefetcher(Cache* cache);
	virtual ~AbstractPrefetcher();
	/*
	 * will be invoked on each miss to check if a prefetch
	 * is required and if so, do the prefetch
	 */
	virtual void doPrefetch(uint32_t addr) = 0;
};

#endif
