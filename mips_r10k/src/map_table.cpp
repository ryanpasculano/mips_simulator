/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include "util.h"
#include "map_table.h"

MapTable::MapTable() {
	DPRINTF(DEBUG_PIPE,
			"MapTable Init \n");
	for (int i = 0; i < NUM_NAMES; i++) {
		table[i].archReg = i;
		table[i].pReg = i;
		table[i].ready = true;
	}
}

MapTable::~MapTable() {
	delete[] table;
}

void MapTable::addMap(uint32_t archReg, uint32_t pReg) {
	for (int i = 0; i < NUM_NAMES; i++) {
		if (table[i].archReg == archReg) {
			table[i].pReg = pReg;
			table[i].ready = false;
		}
	}
}

void MapTable::setReady(uint32_t archReg) {
	for (int i = 0; i < NUM_NAMES; i++) {
		if (table[i].archReg == archReg) {
			table[i].ready = true;
		}
	}
}

bool MapTable::getReady(uint32_t archReg) {
	for (int i = 0; i < NUM_NAMES; i++) {
		if (table[i].archReg == archReg) {
			return table[i].ready;
		}
	}
}

int MapTable::getPRegId(uint32_t archReg) {
	if (archReg < 0 || archReg >= NUM_NAMES){
		return -1;
	}
	for (int i = 0; i < NUM_NAMES; i++) {
		if (table[i].archReg == archReg) {
			return table[i].pReg;
		}
	}
}
void MapTable::swapPReg(uint32_t oldPreg, uint32_t newPreg) {
	for (int i = 0; i < NUM_NAMES; i++) {
		if (table[i].pReg == oldPreg) {
			table[i].pReg = newPreg;
		}
	}
 }