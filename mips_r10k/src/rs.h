/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */
 /*
  * Reservation Station structure
  */

#ifndef __RS__
#define __RS__
#include <iostream>
#include <string>
#include <cstdint>
#include <util.h>
#include <vector>
using namespace std;
#define MAX_RS_SLOTS 6
 


class RS {


public:

	typedef struct rsRow {
		int FU;
		bool busy;
		int destTag;
		int src1;
		bool src1Ready;
		int src2;
		bool src2Ready;

	};
	vector<rsRow> rows;
	RS();
	~RS();

	bool canAllocate(int FU);
	int assignSlot(int FU, int dest, int src1, bool src1ready, int src2, bool src2ready);
	void removeSlot(int FU);
	void updateSlot(int tag, bool ready);
	bool isReady(int FU);
	void freeIssued();
	bool getBusy(int i);
	friend ostream& operator << (ostream &out, const RS & r);

};

#endif
