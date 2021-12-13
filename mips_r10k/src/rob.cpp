
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include "rob.h"
#include "util.h"
#include <vector>

//Function to initialize ROB
ROB:: ROB (){

	/*for (int i = 0; i < ROB_SIZE; i++) {
		
		robROWS.at(i).dispatched = false;
		robROWS.at(i).pc = -1;
		robROWS.at(i).T = -1;
		robROWS.at(i).T_old = -1;
	}*/
} 

//Function to print states in ROB for particular instruction
// void ROB::robPrintState(Pipe_Op *op){
// 	printf("Printing ROB \n");
// 	for(int i = 0; i< ROB_SIZE; i++) {
//     printf(" %d\n", robROWS[i].T);
// 	printf(" %d\n", robROWS[i].T_old);
// 	printf(" %d\n" , robROWS[i].op);
//   }
//   printf("\n");
// }

bool ROB::canAllocate(){
	if (robROWS.size() >= ROB_SIZE){
		return false;
	} else {
		return true;
	}
}
 	
//Function to add instruction in ROB
//Call this function in dispatch stage in pipe to allocate
bool ROB::allocate(uint32_t T,uint32_t T_old, uint32_t pc){
	
	robROW r = {T,T_old,pc, false};
	robROWS.push_back(r);
	return true;
}

robROW ROB::getHead(){
	return robROWS.front();	
}

int ROB::getSize(){
	return robROWS.size();
}

void ROB::setDispatchedTrue(int id) {
	robROWS.at(id).dispatched = true;
}
bool ROB::isDispatched(int id)
{
	return robROWS.at(id).dispatched;
}

robROW ROB::getROBrow(int id)
{
	return robROWS.at(id);
}

int ROB::findByPC(int pc)
{
	for (int i = 0; i < robROWS.size(); i++)
	{
		if (robROWS.at(i).pc == pc)
		{
			return i;
		}
	}
	return -1;
}

//Function to update head pointer when instruction is complete
bool ROB::robUpdateHead(){
	robROWS.erase(robROWS.begin());
	return true;
}

// Flush when branch mis-speculated

// bool ROB::robFlushInst(Pipe_Op *op){

// //TODO: Waiting for branch mask
// 	if ()
// 	{
// 		// for branch mis-speculation
// 		int i;
// 		for (i=robROWS.size(); i> 0; i--){
// 			if(robROWS[i].branchIndex == 1){
// 				tail = i;
// 				index = i;
// 			}
// 		}
// 	}
// 	else
// 	{
// 		//rollback till head pointer for other errors
// 		tail = head;
// 		index = head;
// 	}
	
// }







