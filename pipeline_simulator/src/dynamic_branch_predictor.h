/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_DYNAMIC_BRANCH_PREDICTOR_H_
#define SRC_DYNAMIC_BRANCH_PREDICTOR_H_

#include "abstract_branch_predictor.h"
#include <vector>

class DynamicBranchPredictor: public AbstractBranchPredictor {
public:
	DynamicBranchPredictor(uint32_t _ras_size, uint32_t _bht_entries, 
		uint32_t _bht_entry_width, uint32_t _pht_width);
	 ~DynamicBranchPredictor();
	uint32_t getTarget(uint32_t PC, int is_branch, int branch_cond);
	void update(uint32_t PC, bool taken, uint32_t target, int is_branch, int branch_cond) ;
private:
	uint32_t ras_size, bht_entries, bht_entry_width, pht_width;

	// helper functions
	bool wasLocalCorrect(uint32_t PC, bool taken);
	bool wasGlobalCorrect(uint32_t PC, bool taken);
	int increment(int value);
	int decrement(int value);

	// data structures	
	// see picture in report for what the letters mean
	std::vector<uint32_t> ras;
	std::vector<int> bht;		//A
	std::vector<int> pht_for_bht;	//B
	std::vector<int> pht_for_gbhr;	//C
	std::vector<uint32_t> btb;	
	std::vector<int> meta_predictor;//D
	
	// global branch history register
	int gbhr;			// history of last n branches

	// constanst based on config
	int bht_mask;			// mask to index bht
	int pht_mask;			// mask to update bht entry and gbhr
	int threshold;			// lowest taken value
	int counter_max;		// max taken value
	int btb_tag_shift;

};

#endif /* SRC_DYNAMIC_NT_BRANCH_PREDICTOR_H_ */
