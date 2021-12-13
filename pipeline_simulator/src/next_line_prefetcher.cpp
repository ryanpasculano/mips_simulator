/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "next_line_prefetcher.h"
#include "cache.h"

NextLinePrefetcher::NextLinePrefetcher(Cache* cache) :
		AbstractPrefetcher(cache) {
	
}

NextLinePrefetcher::~NextLinePrefetcher() {

}

/*You should complete this function*/
void NextLinePrefetcher::doPrefetch(uint32_t addr){
//	Packet * newPkt = new Packet(true, false, PacketTypePrefetch, addr + 64, 
//					cache -> getBlkSize(), pkt->data, currCycle, false);
//	cache -> sendReq(pkt);
/*	while(cache){
			prefetch(cache->next);
			//Get the next cache line: TODO what can be the value of N ?
			
			
			cache = cache->next;	
			//TODO save the next N cachelines in a prefetch buffer or cache.
		}
	*/	
	return;
}

	

