/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "abstract_branch_predictor.h"


AbstractBranchPredictor::AbstractBranchPredictor() {
	// TODO Auto-generated constructor stub
	
}

AbstractBranchPredictor::~AbstractBranchPredictor() {
	// TODO Auto-generated destructor stub
}

uint32_t AbstractBranchPredictor::getTarget(uint32_t PC, int is_branch, int branch_cond) {

	return -1;
}

void AbstractBranchPredictor::update(uint32_t PC, bool taken, uint32_t target, int is_branch, int branch_cond) {
	//no metadata to update:D
	return;
}

