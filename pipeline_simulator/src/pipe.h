/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __PIPE_H__
#define __PIPE_H__

#include <cstdint>

#include "abstract_branch_predictor.h"
#include "abstract_memory.h"
#include "base_object.h"

/* Pipeline ops (instances of this structure) are high-level representations of
 * the instructions that actually flow through the pipeline. This struct does
 * not correspond 1-to-1 with the control signals that would actually pass
 * through the pipeline. Rather, it carries the original instruction, operand
 * information and values as they are collected, and destination information. */
typedef struct Pipe_Op {
	//PC of this instruction
	uint32_t pc;
	//raw instruction */
	uint32_t instruction;
	//decoded opcode and subopcode fields
	int opcode, subop;
	//immediate value, if any, for ALU immediates
	uint32_t imm16, se_imm16;
	//shift amount
	int shamt;
	//register source values
	int reg_src1, reg_src2; /* 0 -- 31 if this inst has register source(s), or
	 -1 otherwise */
	uint32_t reg_src1_value, reg_src2_value; /* values of operands from source
	 regs */

	//memory access information
	int is_mem; /* is this a load/store? */
	uint32_t mem_addr; /* address if applicable */
	int mem_write; /* is this a write to memory? */
	uint32_t mem_value; /* value loaded from memory or to be written to memory */

	//register destination information
	int reg_dst; /* 0 -- 31 if this inst has a destination register, -1
	 otherwise */
	uint32_t reg_dst_value; /* value to write into dest reg. */
	int reg_dst_value_ready; /* destination value produced yet? */

	//branch information
	int is_branch; /* is this a branch? */
	uint32_t branch_dest; /* branch destination (if taken) */
	int branch_cond; /* is this a conditional branch? */
	int branch_taken; /* branch taken? (set as soon as resolved: in decode
	 for unconditional, execute for conditional) */
	int is_link; /* jump-and-link or branch-and-link inst? */
	int link_reg; /* register to place link into? */
	//multiplier stall info
	int stall;

	//fetch operation info
	bool isFetchIssued;
	Packet* instFetchPkt;

	//memory operation info
	bool waitOnPktIssue;
	bool memTried;
	Packet* memPkt;

	//shows if the operation is ready for the next stage of pipeline
	bool readyForNextStage;
} Pipe_Op;

/* The pipe state represents the current state of the pipeline. It holds a
 * pointer to the op that is currently at the input of each stage. As stages
 * execute, they remove the op from their input (set the pointer to NULL) and
 * place an op at their output. If the pointer that represents a stage's output
 * is not null when that stage executes, then this represents a pipeline stall,
 * and the stage must not overwrite its output (otherwise an instruction would
 * be lost).
 */

class PipeState: public BaseObject {
public:
	PipeState();
	PipeState(uint32_t ras_size, uint32_t bht_entries, uint32_t bht_entry_width, uint32_t pht_width);
	~PipeState();
	//pipe op currently at the input of the given stage (NULL for none)
	Pipe_Op *fetch_op, *decode_op, *execute_op, *mem_op, *wb_op;

	//pointer to the branch predictor
	AbstractBranchPredictor* BP;

	//register file state
	uint32_t REGS[32];
	uint32_t HI, LO;

	//program counter in fetch stage
	uint32_t PC;

	/* information for PC update (branch recovery). Branches should use this
	 * mechanism to redirect the fetch stage, and flush the ops that came after
	 * the branch as necessary. */
	int branch_recover; /* set to '1' to load a new PC */
	uint32_t branch_dest; /* next fetch will be from this PC */
	int branch_flush; /* how many stages to flush during recover? (1 = fetch, 2 = fetch/decode, ...) */

	//if the simulator should keep running
	int RUN_BIT;

	//pointers to the first level of memory hierarchy
	AbstractMemory* data_mem;
	AbstractMemory* inst_mem;

	//statistics
	uint32_t stat_cycles;
	uint32_t stat_inst_retire;
	uint32_t stat_inst_fetch;
	uint32_t stat_squash;

	//this function calls the others
	void pipeCycle();

	/* helper: pipe stages can call this to schedule a branch recovery */
	/* flushes 'flush' stages (1 = execute only, 2 = fetch/decode, ...) and then
	 * sets the fetch PC to the given destination. */
	void pipeRecover(int flush, uint32_t dest);

	//each of these functions implements one stage of the pipeline
	void pipeStageFetch();
	void pipeStageDecode();
	void pipeStageExecute();
	void pipeStageMem();
	void pipeStageWb();

	//send a memory operation request
	virtual bool sendReq(Packet * pkt) override;
	//receives a response to the request for a memory operation
	virtual void recvResp(Packet* readRespPkt) override;

	// place other information here as necessary

};

#endif
