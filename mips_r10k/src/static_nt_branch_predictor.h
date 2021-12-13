/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_STATIC_NT_BRANCH_PREDICTOR_H_
#define SRC_STATIC_NT_BRANCH_PREDICTOR_H_

#include "abstract_branch_predictor.h"
/*
 * Static Not-Taken branch predictor
 */
class StaticNTBranchPredictor: public AbstractBranchPredictor {
public:
	StaticNTBranchPredictor();
	virtual ~StaticNTBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) override;
	virtual void update(uint32_t PC, bool taken, uint32_t target) override;
};

#endif /* SRC_STATIC_NT_BRANCH_PREDICTOR_H_ */
