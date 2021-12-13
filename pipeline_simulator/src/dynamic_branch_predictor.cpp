/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "dynamic_branch_predictor.h"
#include <math.h>
#include <cstdio>
#include <vector>

DynamicBranchPredictor::DynamicBranchPredictor(uint32_t _ras_size, uint32_t _bht_entries, 
		uint32_t _bht_entry_width, uint32_t _pht_width) :
		AbstractBranchPredictor(), ras_size(_ras_size), bht_entries(_bht_entries),
		bht_entry_width(_bht_entry_width), pht_width(_pht_width) {

	ras.resize(ras_size);
	bht.resize(bht_entries);
	pht_for_bht.resize(pow(2, bht_entry_width));
	pht_for_gbhr.resize(pow(2, bht_entry_width));
	btb.resize(bht_entries);
	meta_predictor.resize(pow(2, bht_entry_width));
	gbhr = 0;
	bht_mask = bht_entries-1;
	pht_mask = pow(2, bht_entry_width) -1;
	threshold = pow(2, pht_width-1);
	counter_max = pow(2, pht_width) - 1;
	btb_tag_shift = int(log2(bht_entries)) + 2;
}

DynamicBranchPredictor::~DynamicBranchPredictor() {
	
}

uint32_t DynamicBranchPredictor::getTarget(uint32_t PC, int is_branch, int branch_cond) {
	uint32_t next_PC = PC + 4;
	// if not a branch return PC+4
	if (is_branch == 1) {
		// it is a branch now check branch_cond
		if (branch_cond == 1) {
			// conditional branch use predictor
		
			// get local prediction
			int bht_index = (PC >> 2) & bht_mask;
			int pht_for_bht_index = bht.at(bht_index);
			int pht_for_bht_value = pht_for_bht.at(pht_for_bht_index);
			
			//get global preditcion
			int pht_for_gbhr_value = pht_for_gbhr.at(gbhr);
			
			//get metapredictor value
			int meta_predictor_index = (PC >> 2) & pht_mask;
			int meta_prediction_value = meta_predictor.at(meta_predictor_index);

			// select which prediction to use
			int prediction_value;
			if (meta_prediction_value < threshold){ // this is arbitrary
				prediction_value = pht_for_bht_value;
			} else {
				prediction_value = pht_for_gbhr_value;
			}

			// set prediction
			if (prediction_value < threshold) {
				// do nothing
			} else {
				// compare tags of PC and btb entry
				if (PC >> btb_tag_shift == btb.at(bht_index) >> btb_tag_shift){
					next_PC = btb.at(bht_index);
				} // else aliasing issue don't use btb value
			}
		} else {
			// unconditional branch use ras
			if (!ras.empty()){
				next_PC = ras.back();
				ras.pop_back();
			}			
		}
	}
	return next_PC;

	return -1;
}

void DynamicBranchPredictor::update(uint32_t PC, bool taken, uint32_t target, int is_branch, int branch_cond) {
	//  
	if (is_branch ==1){
		if (branch_cond ==1){
			// get outcome of each predictor
			bool local_correct = wasLocalCorrect(PC, taken);
			bool global_correct = wasGlobalCorrect(PC, taken);
			
			//get metapredictor value
			int meta_predictor_index = (PC >> 2) & pht_mask;
			int meta_prediction = meta_predictor.at(meta_predictor_index);

			//update metapredictor
			if (local_correct && !global_correct){
				//decreement metapredictor
				meta_prediction = decrement(meta_prediction);	
			} else if (!local_correct && global_correct){
				// increment metapredictor
				meta_prediction = increment(meta_prediction);	
			}
			meta_predictor.at(meta_predictor_index) = meta_prediction;

			// update local predictor		
			int bht_index = (PC >> 2) & bht_mask;
			int pht_for_bht_index = bht.at(bht_index);
			int pht_for_bht_value = pht_for_bht.at(pht_for_bht_index);

			// updating pht
			if(taken){
				// increment counter
				pht_for_bht_value = increment(pht_for_bht_value);
			} else { 
				// decrement counter
				pht_for_bht_value = decrement(pht_for_bht_value);
			}
			pht_for_bht.at(pht_for_bht_index) = pht_for_bht_value;

			// updating bht
			pht_for_bht_index = (pht_for_bht_index << 1);
			if (taken) {
				pht_for_bht_index++;
			} 
			pht_for_bht_index = pht_for_bht_index & pht_mask;
			bht.at(bht_index) = pht_for_bht_index;
			
			// update global predictor
			int pht_for_gbhr_value = pht_for_gbhr.at(gbhr);
			
			// updating pht
			if(taken){
				// increment counter
				pht_for_gbhr_value = increment(pht_for_gbhr_value);
			} else { 
				// decrement counter
				pht_for_gbhr_value = decrement(pht_for_gbhr_value);
			}
			// update gbhr
			gbhr = gbhr << 1;
			if (taken){
				gbhr++;
			}
			gbhr = gbhr & pht_mask;

			// update btb
			if (taken){
				btb.at(bht_index) = target;
			}
			
		} else {
			//unconditional branch
			if (ras.size() ==  ras.max_size()){
				ras.erase(ras.begin());
			}
			ras.push_back(target);
		}
	}

}

bool DynamicBranchPredictor::wasGlobalCorrect(uint32_t PC, bool taken){
	// get global prediction
	int pht_for_gbhr_value = pht_for_gbhr.at(gbhr);

	// check if correct
	if (!taken && pht_for_gbhr_value < threshold) { 
		return true;
	} else if (taken && pht_for_gbhr_value >= threshold) {
		return true;
	} else {
		return false;
	}
}

bool DynamicBranchPredictor::wasLocalCorrect(uint32_t PC, bool taken){
	
	// get local prediction	
	int bht_index = (PC >> 2) & bht_mask;
	int pht_for_bht_index = bht.at(bht_index);
	int pht_for_bht_value = pht_for_bht.at(pht_for_bht_index);

	// check if correct
	if (!taken && pht_for_bht_value < threshold) { 
		return true;
	} else if (taken && pht_for_bht_value >= threshold) {
		return true;
	} else {
		return false;
	}

}

int DynamicBranchPredictor::increment(int value){

	if(value != counter_max) {
		// increment counter
		value++;
	} 
	return value;
}

int DynamicBranchPredictor::decrement(int value){

	if (value != 0) { 
		// decrement counter
		value--;
	}
	return value;
}
