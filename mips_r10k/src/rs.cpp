/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
using namespace std;
#include <string>
#include <stdlib.h>
#include "util.h"
#include "rs.h"


RS::RS(){
	for (int i = 0; i < MAX_RS_SLOTS; i++) {
		rsRow r = {i, false, -1, -1, false, -1, false};	
		rows.push_back(r);
	}
}

RS::~RS() {
	
}

bool RS::canAllocate(int FU)
{
	int x = rows.size();
	int y;
	if (FU == FP1 or FU == FP2) {
		if (rows.at(FP1).busy == false || rows.at(FP2).busy == false) {
			return true; // one of the FP slots available
		}
	} else if (FU == ALU1 or FU == ALU2) { 
		if (rows.at(ALU1).busy == false || rows.at(ALU2).busy == false) {
			return true; // one of the ALU slots available
		}
	} else if (rows.at(FU).busy == false) {
		return true; // slot is free
	}
	return false; //slot not free
}


int RS::assignSlot(int FU, int dest, int src1, bool src1ready, int src2, bool src2ready)
{
	if (FU == FP1 and rows.at(FU).busy == true) FU = FP2; 
	else if (FU == FP2 and rows.at(FU).busy == true) FU = FP1;
	else if (FU == ALU1 and rows.at(FU).busy == true) FU = ALU2;
	else if (FU == ALU2 and rows.at(FU).busy == true) FU = ALU1;
	if (rows.at(FU).busy == false) {
		rows.at(FU).destTag = dest;
		rows.at(FU).src1 = src1;
		rows.at(FU).src1Ready = src1ready;
		rows.at(FU).src2 = src2;
		rows.at(FU).src2Ready = src2ready;
		rows.at(FU).busy = true;
		return FU; //slot assignment successful
	}
	return -1; //slot not free (should never happen cz we only call this function when a slot is free)
}

void RS::removeSlot(int FU)
{
	rows.at(FU).busy = false;
}

//use this as a broadcast receiver
void RS::updateSlot(int tag, bool ready)
{
	for(int i=0; i<MAX_RS_SLOTS; i++){
		if (rows.at(i).src1 == tag) {
			rows.at(i).src1Ready = ready;
		}
		else if (rows.at(i).src2 == tag) {
			rows.at(i).src2Ready = ready;
		}	
	
	}
}

bool RS::isReady(int FU)
{
	if (rows.at(FU).src1Ready and rows.at(FU).src2Ready) {
		return true;
	} else {
		return false;
	}

}

void RS::freeIssued(){
	for (int i = 0; i < MAX_RS_SLOTS; i++)
	{
		if (rows.at(i).src1Ready and rows.at(i).src2Ready) 
		{
			removeSlot(i);
		}
	}
}

bool RS::getBusy(int i){
	return rows.at(i).busy;
}

ostream & operator << (ostream &out, const RS & r) {
	RS r1 = r;
	for(int i=0; i<MAX_RS_SLOTS; i++){
		//rsROW row = r.getRow(i);
		out << "Slot:" << i << " Busy: " << r1.getBusy(i)  << endl;
	}
	return out;
}

