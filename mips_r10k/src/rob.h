
/* ROB */

#ifndef _ROB_H_
#define _ROB_H_
#include <cstdint>
#include <cstdlib>
#include <assert.h>
#include <vector>
#include "abstract_memory.h"
#define ROB_SIZE 30 
using namespace std;
//constraining the size to 30

typedef struct robROW {
	uint32_t T;
	uint32_t T_old;
	uint32_t pc;
	bool dispatched;
	uint32_t mask;
};

class ROB{

	public:
		//defining data-type for ROB as vector
		//robROW *robROWS = new robROW[ROB_SIZE];
		vector<robROW> robROWS;
		
		//Function to initialize ROB
		ROB();

		//Function to print states in ROB for particular instruction
		// void robPrintState(Pipe_Op *op);
		//if theres sapce in ROB function
		bool canAllocate();

		//Function to add instruction in ROB and update tail pointer
		bool allocate(uint32_t, uint32_t, uint32_t); // this instruction sets the head and tail pointers

		//Function to update head pointer when the function retires
		bool robUpdateHead();

		//Function to remove all the younger-instruction during branch mis-speculation.
		bool robFlushInst(uint32_t);

		//Function to get the information from the head of ROB
		robROW getHead();

		// getter for size of rob
		int getSize();
		int findByPC(int pc);

		//function to see if an instruction in the rob has been dipatched
		bool isDispatched(int id);
		void setDispatchedTrue(int id);

		//function to get the info at an index
		robROW getROBrow(int id);
	
};




#endif
