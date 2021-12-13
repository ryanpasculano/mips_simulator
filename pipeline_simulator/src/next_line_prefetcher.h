/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __NEXT_LINE_PREFETCHER_H__
#define __NEXT_LINE_PREFETCHER_H__

/*
 * You should impelement the next-line prefetcher
 */
#include "abstract_prefetcher.h"

class NextLinePrefetcher: public AbstractPrefetcher {
public:
	NextLinePrefetcher(Cache* cache);
	virtual ~NextLinePrefetcher();
	virtual void doPrefetch(uint32_t addr) override;
	//place other functions here if necessary
	//Function to store prefetched cachline in a prefetch buffer.
private:
	//Initialize buffer to store prefetched cachlines.

};

#endif
