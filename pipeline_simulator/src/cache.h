/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __CACHE_H__
#define __CACHE_H__

#include "block.h"
#include "abstract_memory.h"
#include "abstract_prefetcher.h"
#include "repl_policy.h"
#include <cstdint>
#include <vector>
#include <queue>
#include <deque>
#include <list>

/*
 * You should implement MSHR
 */
class MSHR {
public:
	MSHR(uint32_t _entries, uint32_t subentries);
	~MSHR();
	bool addPacket(Packet * pkt); 
	Packet* getPacket(int addr);
	void printMSHR();

private:
	uint32_t entries, subentries;
	std::deque<std::deque<Packet *>> mshr;
        	
};
//Defining write back buffer
class WBB {

public:
	WBB(uint32_t packetSize);
	~WBB();
	bool addPacket(Packet * pkt); 
	bool delPacket(Packet * pkt);
	Packet* findPacket(uint32_t addr);
private:
	int wbbCapacity;
	int packetSize;
	std::deque <Packet*> wbb;
};
/*
 * You should implement Cache
 */
class Cache: public AbstractMemory {
private:
	AbstarctReplacementPolicy *replPolicy;
	AbstractPrefetcher* prefetcher;
	MSHR* mshr;
	uint32_t cSize, associativity, blkSize, numSets, wbb_entries, writeThrough, writeBack, lastUsed;
	WBB* wbb;
	char* name;

public:
	//Pointer to an array of block pointers
	Block ***blocks;
	Cache(uint32_t _Size, uint32_t _associativity, uint32_t _blkSize,
			enum ReplacementPolicy _replPolicy, uint32_t _delay,
			uint32_t _mshr_entries, uint32_t _mshr_subentries, uint32_t packetSize, uint32_t _writeThrough, uint32_t _writeBack, char* _name);
	~Cache();
	bool sendReq(Packet * pkt) override;
	virtual void recvResp(Packet* readRespPkt) override;
	virtual void Tick() override;
	int getWay(uint32_t addr);
	virtual uint32_t getAssociativity();
	virtual uint32_t getNumSets();
	virtual uint32_t getBlockSize();
	/*
	 * read the data if it is in the cache. If it is not, read from memory.
	 * this is not a normal read operation, this is for debug, do not use
	 * mshr for implementing this 
	 */
	virtual void dumpRead(uint32_t addr, uint32_t size, uint8_t* data) override;
	//place other functions here if necessary
	int getIndex(uint32_t addr);
	int getFree(uint32_t addr);

};


#endif
