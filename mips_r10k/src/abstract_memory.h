/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __ABSTRACT_MEMORY_H__
#define __ABSTRACT_MEMORY_H__
#include<cstdint>
#include <queue>
#include "util.h"
#include "base_object.h"


/*
 * AbstractMemory should be inherited by BaseMemory and Cache
 */
class AbstractMemory : public BaseObject {
public:
	AbstractMemory(uint32_t reqQueueCapacity);
	virtual ~AbstractMemory();
	/*
	 * invoked each cycle for this memory object
	 * to service and respond the packets that are
	 * ready to be serviced
	 */
	virtual void Tick() = 0;

	//send a request to this memory object
	virtual bool sendReq(Packet * pkt) = 0;

	//this memory object has received a packet
	virtual void recvResp(Packet* readRespPkt) = 0;
	//request queue for incoming requests
	std::queue<Packet*> reqQueue;
	//max capacity of request queue
	uint32_t reqQueueCapacity;

	

	/*
	 * pointer to next and previous base objects.
	 * for example: in memory next points to null
	 * and prev points to core (you should change this
	 * since you are adding the caches)
	 */
	BaseObject* next;
	BaseObject* prev;

	/*
	 * function for reading a portion of memory and used for
	 * comparing your design with baseline. You should correctly
	 * implement this for caches so at the end of simulation
	 * the memdump function will retrieve the latest value of
	 * the portion of memory that has been modified
	 */
	virtual void dumpRead(uint32_t addr, uint32_t size, uint8_t* data) = 0;
};

#endif
