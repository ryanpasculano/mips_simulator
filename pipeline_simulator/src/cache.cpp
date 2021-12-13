
 /*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdlib>
#include <vector>
#include <iostream>
#include "repl_policy.h"
#include "next_line_prefetcher.h"
#include <memory.h>
#include "cache.h"
#include "block.h"
#include <queue>
using namespace std;

Cache::Cache(uint32_t _size, uint32_t _associativity, uint32_t _blkSize,
		enum ReplacementPolicy _replType, uint32_t _delay,
		uint32_t _mshr_entries, uint32_t _mshr_subentries, uint32_t packetSize, uint32_t _writeThrough, uint32_t _writeBack, char* _name):
		AbstractMemory(_delay, 100), cSize(_size),
		associativity(_associativity), blkSize(_blkSize), writeThrough(_writeThrough), writeBack(_writeBack), name(_name){

	lastUsed = 0;
	numSets = cSize / (blkSize * associativity);
	blocks = new Block**[numSets];
	for (int i = 0; i < (int) numSets; i++) {
		blocks[i] = new Block*[associativity];
		for (int j = 0; j < associativity; j++)
			blocks[i][j] = new Block(blkSize);
	}

	switch (_replType) {
	case RandomReplPolicy:
		replPolicy = new RandomRepl(this);
		break;
	case LRUReplPolicy:
		replPolicy = new LRURepl(this);
		break;
	case PLRUReplPolicy:
		replPolicy = new PLRURepl(this);
		break;
	default:
		assert(false && "Unknown Replacement Policy");
	}

	// create mshr
	mshr = new MSHR(_mshr_entries, _mshr_subentries);

	// create prefetcher
	prefetcher = new NextLinePrefetcher(this);

	//create wbb	
	wbb = new WBB(packetSize);

}

Cache::~Cache() {
	delete replPolicy;
	for (int i = 0; i < (int) numSets; i++) {
		for (int j = 0; j < associativity; j++)
			delete blocks[i][j];
		delete blocks[i];
	}
	delete blocks;
	delete mshr;
	delete wbb;

}

uint32_t Cache::getAssociativity() {
	return associativity;
}

uint32_t Cache::getNumSets() {
	return numSets;
}

uint32_t Cache::getBlockSize() {
	return blkSize;
}
//getting index for writes and reads
int Cache::getWay(uint32_t addr) {
	uint32_t _addr = addr / blkSize;
	uint32_t setIndex = (numSets - 1) & _addr;
	uint32_t addrTag = _addr / numSets;
	for (int i = 0; i < (int) associativity; i++) {
		if ((blocks[setIndex][i]->getValid() == true)
				&& (blocks[setIndex][i]->getTag() == addrTag)) {
			return i;
		}
	}
	return -1;
}
//getting index 
int Cache::getIndex(uint32_t addr) {
	uint32_t _addr = addr / blkSize;
	uint32_t setIndex = (numSets - 1) & _addr;
	uint32_t addrTag = (_addr / numSets);
	for (int i = 0; i < (int) associativity; i++) {
		if (blocks[setIndex][i]->getTag() == addrTag) {
			return i;
		}
	}
	return -1;
}



/*You should complete this function*/
bool Cache::sendReq(Packet * pkt){
	// this is called by  the a smaller memory structure requesting information
	DPRINTF(DEBUG_CACHE,
		"request for cache %s for pkt : addr = %x, type = %d\n",
		name, pkt->addr, pkt->type);
	int i = getWay(pkt->addr);
	if (i != -1) {
		//its a hit
		pkt->ready_time += accessDelay;
		if (reqQueue.size() < reqQueueCapacity) {
			reqQueue.push(pkt);
			//request received now ack
			return true;
		}
	}
	else {
		//cache miss
		pkt->ready_time += accessDelay;

		//add into MSHR		
		bool canAddPkt = mshr->addPacket(pkt);
		// send block sized packet to next cache
		if (canAddPkt){
			Packet * newPkt = new Packet(true, false, PacketTypeFetch, pkt -> addr, 
						blkSize, nullptr, pkt -> ready_time, pkt ->isInstruction);
			
			newPkt -> data = static_cast<uint8_t*>(malloc(sizeof(uint8_t)*blkSize));
			//memcpy(pkt->data, newPkt -> data, blkSize); 
			
			next->sendReq(newPkt);	
			// evoke function to perform doPrefetch()
		}
		return canAddPkt;
		
	}


}

/*You should complete this function*/
void Cache::recvResp(Packet* readRespPkt){
	// a larger cache is sending us something
	DPRINTF(DEBUG_CACHE,
			"cache %s received a response for pkt : addr = %x, type = %d\n",
			name, readRespPkt->addr, readRespPkt->type);
	
	uint32_t setIndex = (numSets - 1) & (readRespPkt->addr / blkSize);
	int victim;	
	uint32_t addr = readRespPkt->addr / blkSize;
	uint32_t addrTag = (addr / numSets);
	/* this is working
	int counter = 0;
	uint8_t* read= readRespPkt->data;
	printf("Cache %s\n", name);
	while(counter < 64 || *(read) != 0){
		printf("%02x\n", *(read));
		read++;
		counter++;
	}
	*/
	
	uint8_t* data = (uint8_t*) malloc(sizeof(uint8_t)* blkSize);
	int counter = 0;
	switch (readRespPkt->type) {
	
		case PacketTypeFetch:

			victim = replPolicy->getVictim(readRespPkt -> addr, readRespPkt -> isWrite);
			if (blocks[setIndex][victim] -> getValid() && blocks[setIndex][victim] -> getDirty()){
				// put in wbb
				Packet * victimPacket = new Packet(true, true, PacketTypeEvict, readRespPkt -> addr, 
						readRespPkt -> size, blocks[setIndex][victim]->getData(), 
						readRespPkt -> ready_time, readRespPkt->isInstruction);
				//victim
				wbb -> addPacket(victimPacket); 
			}
			memcpy(blocks[setIndex][victim], readRespPkt->data, readRespPkt->size); 
			blocks[setIndex][victim]->updateLast(0);
			blocks[setIndex][victim]->clear(addrTag);
			
			//printf("setIndex:%d, i: %d\n", setIndex, victim);
			/*
			data = blocks[setIndex][victim] -> getData();
			printf("Cache %s\n", name);
			while(counter < 64 || *data != 0){
				printf("%02x\n", *data);
				data++;
				counter++;
			}
			*/
			//for (int i = 0; i < blkSize; i += 4){
			//	cout<<blocks[setIndex][victim]<<endl;
			//}
			//find mshr packet for that address (can be more than one packet)
			//push that packet into the reqQueue
			Packet* reqPkt;
			reqPkt = mshr->getPacket(readRespPkt->addr);
			while (reqPkt != nullptr) {
				reqPkt -> ready_time = readRespPkt ->ready_time + accessDelay;
				DPRINTF(DEBUG_CACHE, "pushing packet to reqQueue with ready_time %d\n", reqPkt -> ready_time);
				reqQueue.push(reqPkt);
				reqPkt = mshr->getPacket(readRespPkt->addr);
			}
			break;
		
		case PacketTypeStore:
			//ok
		
			break;
		// if it is confirmation that an eviction block has been written tot he prev cache
		case PacketTypeEvict:
			//remove from wbb
			wbb->delPacket(readRespPkt);
		
			break;
		
		
		default:
			assert(false && "Invalid response from memory or cache");
	}
	
	
	return;
}

/*You should complete this function*/
void Cache::Tick(){

	// service ready queue
	
	while (!reqQueue.empty()) {
		DPRINTF(DEBUG_CACHE, "front of queue ready time: %d\n", reqQueue.front()->ready_time);
		if (reqQueue.front()->ready_time <= currCycle) {
			Packet* respPkt = reqQueue.front();
			reqQueue.pop();
			DPRINTF(DEBUG_CACHE,
				"cache %s send respond for pkt: addr = %x, ready_time = %d\n",
				name, respPkt->addr, respPkt->ready_time);
			//printf("Printing Packet in tick\n");
			//for (int i = 0; i < respPkt ->size; i +=4){
			//	printf("0x%08x\n", *(respPkt->data + i));
			//}
			uint32_t setIndex = (numSets - 1) & (respPkt->addr / blkSize);
			//uint32_t addrTag = respPkt->addr / numSets;
			uint32_t offset = (blkSize - 1) & respPkt->addr;
			//get the blk id
			int i = getWay(respPkt->addr);
			 
			//update last used for LRU and pseudo LRU
			lastUsed++;
			blocks[setIndex][i]->updateLast(lastUsed);
			//write
			if (respPkt->isWrite) {
				memcpy(blocks[setIndex][i] + offset, respPkt->data, respPkt->size);
				

				//write through
				if (writeThrough == 1) {
					// push the 64 byte packet to write back buffer
					wbb->addPacket(respPkt);
				}
							
				//write back
				else if (writeBack == 1) {
					//set dirty bit of the block to 1
					blocks[setIndex][i]->setDirty(true);
				}
				

				//change this pkt to respond pkt
				respPkt->isReq = false;
				//the data part is no longer needed
				delete respPkt->data;
				respPkt->data = nullptr;
				
			}
			//read
			else {
				//perform the read
				uint8_t* fullBlock =(uint8_t*) malloc(sizeof(uint8_t) * blkSize);
				uint8_t* temp = fullBlock;
				memcpy(fullBlock, blocks[setIndex][i], blkSize);
				//int counter = 0;

				//while(counter < blkSize || *fullBlock != 0){
				//	printf("%02x\n", *fullBlock);
				//	fullBlock++;
				//	counter++;
				//}
				fullBlock = temp;
				for (int i = 0; i < offset; i++){
					fullBlock++;
				}
				memcpy(respPkt->data, fullBlock, respPkt->size);
				/*				
				for (uint32_t i = 0; i < respPkt->size; i++) {
					*(respPkt->data + i) = *fullBlock + i;
				}
				*/
				//counter = 0;

				fullBlock = respPkt->data;
				//while(counter < blkSize || *fullBlock != 0){
				//	printf("%02x\n", *fullBlock);
				//	fullBlock++;
				//	counter++;
				//}
				respPkt->isReq = false;
 			}
			/*
			* send the respond to the previous base_object which is waiting
			* for this respond packet.
			*/
			if(respPkt ->isInstruction){
				prev_instruction->recvResp(respPkt);
			} else {
				prev->recvResp(respPkt);
			}			
		}
		else {
			/*
			 * assume that reqQueue is sorted by ready_time for now
			 * (because the pipeline is in-order)
			 */
			break;
		}

		
	}
	return;
}

/*needs to read addr from cache*/
void Cache::dumpRead(uint32_t addr, uint32_t size, uint8_t* data){
	uint32_t setIndex = (numSets - 1) & (addr / blkSize);
	int i = getWay(addr);
	Packet *pkt = wbb->findPacket(addr);
	if (i != -1) {
		//its a cache hit
		memcpy(data, blocks[setIndex][i], size);
	}
	else if (pkt!=nullptr){
		//its a wbb hit
		memcpy(data, pkt, size);
	}
	
}

// MSHR constructor
MSHR::MSHR(uint32_t entries, uint32_t subentries):
	entries(entries), subentries(subentries){
	
	mshr.resize(entries);
	
}

// MSHR deconstructor
MSHR::~MSHR(){


}

/* 
 * adds packet to MSHR if there is space and returns true
 * if there is no space then returns false
 */
bool MSHR::addPacket(Packet * pkt){
	DPRINTF(DEBUG_CACHE,
		"MSHR addPacket pkt : addr = %x, type = %d\n",
		pkt->addr, pkt->type);	
	int maxRows = entries;
	if (mshr.size() < maxRows) {
		maxRows = mshr.size();
	}

	// iterate through mshr and add it if you can
	for (int i = 0; i < maxRows; i++){
		//mshr.at(i).resize(subentries);
		if (mshr.at(i).empty()){
			mshr.at(i).push_back(pkt);
			return true;
		}else if (mshr.at(i).size() == 0) {
			// place in first free spot
			mshr.at(i).push_back(pkt);
			return true;
		}else if (pkt->addr == mshr.at(i).at(0) -> addr && mshr.at(i).size() < subentries){
			//if there is room in the subentries find first available subentry
			mshr.at(i).push_back(pkt);
			return true;
		}  
	}
	return false;

}

/*
 * returns the first packet withe address addr 
 * and shifts the MSHR as needed
 * returns NULL if no packet found with addr
 */
Packet* MSHR::getPacket(int addr){
	DPRINTF(DEBUG_CACHE,
		"MSHR getPacket, looking for pkt : addr = %x\n",
		addr);	
	// iterate through the mshr and return a packet if the address matches
	// shift items around as necesarry
	mshr.resize(entries);
	int maxRows = entries;
	if (mshr.size() < maxRows) {
		maxRows = mshr.size();
	}

	for (int i = 0; i < maxRows; i++){
		if (mshr.at(i).size() !=0 && mshr.at(i).at(0)->addr == addr){
			Packet * pkt = mshr.at(i).front();
			mshr.at(i).erase(mshr.at(i).begin());
			return pkt;
		}

	}

	return nullptr;
}

void MSHR::printMSHR(){
	DPRINTF(DEBUG_CACHE,
		"MSHR print\n");	
	// iterate through mshr and add it if you can
	int maxRows = entries;
	if (mshr.size() < maxRows) {
		maxRows = mshr.size();
	}

	for (int i = 0; i < maxRows; i++){
		if (mshr.at(i).size() != 0) {
			//if there is room in the subentries find first available subentry
		}  
	}

}


// Write Back Buffer (wbb) constructor function
WBB::WBB(uint32_t wbbCapacity):
	   packetSize(packetSize){
	   std::queue <Packet*> wbb;
}


// WBB destructor
WBB::~WBB(){
	
}

// Adding the packets to wbb from cache
bool WBB::addPacket (Packet* pkt){
	//check if wbb is full using NULL 
	if ( wbb.size() < wbbCapacity ){
		wbb.push_back(pkt);	
		DPRINTF(DEBUG_CACHE,
		"Packet added in write back buffer for pkt : addr = %x, type = %d\n",
		pkt->addr, pkt->type);
	// return true when the packet has been stored
		return true;
	} else{
	// return false since the wbb could not add the packet because of the buffer being full
	return false;
	}		
	//If wbbQueue is full ,stall using readyTime.	
}


bool WBB::delPacket (Packet* pkt){
	//Check if the wbb is empty
	if ( wbb.size() == 0 ){
	   DPRINTF(DEBUG_CACHE,
		"write back buffer empty, deleted pkt : addr = %x, type = %d\n",
		pkt->addr, pkt->type);	
	}
	//Remove the packet from wbb
	else{
	   wbb.pop_front();	
	}
}

Packet* WBB::findPacket(uint32_t addr){
	int maxRows = wbbCapacity;
	if (wbb.size() < maxRows) {
		maxRows = wbb.size();
	}

	for (int i = 0; i < maxRows; i++){
		if (wbb.at(i)->addr == addr) {
			return wbb.at(i);
		}  
	}
	return nullptr;
}

// Dirty bit -> in L1 cache, evict the packet to wbb and write to L2.'


//TODO define next line prefetch function here ? Call function when add->type == fetch 




