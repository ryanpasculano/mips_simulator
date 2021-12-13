/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __PIPE_H__
#define __PIPE_H__

#include <cstdint>
#include <vector>
using namespace std;

#include "abstract_branch_predictor.h"
#include "abstract_memory.h"
#include "base_object.h"
#include "map_table.h"
#include "rs.h"
#include "rob.h"
#include "lsq.h"
#include "freelist.h"


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

	// shows which FU it is assigned to
	int FU;

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
	PipeState(uint32_t rob_size, uint32_t lsq_size);
	~PipeState();

	//pipe op currently at the input of the given stage (NULL for none)
	
	vector<Pipe_Op*> fetch_ops;
	vector<Pipe_Op*> dispatch_ops;
	vector<Pipe_Op*> issue_ops;
	vector<Pipe_Op*> execute_ops;
	vector<Pipe_Op*> complete_ops;
	vector<Pipe_Op*> retire_ops;
	///legacy  please delete soon
	Pipe_Op *fetch_op, *decode_op, *dispatch_op, *execute_op, *mem_op, *wb_op;

	//pointer to the branch predictor
	AbstractBranchPredictor* BP;

	//pointer to the Map Table
	MapTable *mapTable;
	//pointer to the Arch Map Table
	MapTable *archMapTable;
	//pointer to the Free List
	FreeList *freeList;
	//pointer to the LSQ	
	LSQ *lsq;
	//pointer to the RS
	RS *rs;
	//pointer to ROB
	ROB *rob;
	

	//register file state
	uint32_t REGS[64];
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
	void pipeRecover(Pipe_Op *op);

	//each of these functions implements one stage of the pipeline
	void pipeStageFetch();
	void pipeStageDispatch();
	void pipeStageIssue();
	void pipeStageExecute();
	void pipeStageComplete();
	void pipeStageRetire();
	//void pipeStageMem();
	//void pipeStageWb();
	void printPipeState();

	//send a memory operation request
	virtual bool sendReq(Packet * pkt) override;
	//receives a response to the request for a memory operation
	virtual void recvResp(Packet* readRespPkt) override;

	// place other information here as necessary

	// all decode functionality from the old PipeStateDecode
	void decode(Pipe_Op *op);
};

#endif
