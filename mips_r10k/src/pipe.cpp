/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "pipe.h"
#include "mips.h"
#include "abstract_memory.h"
#include "static_nt_branch_predictor.h"
#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "util.h"
#include <vector>
using namespace std;

/* debug */
void printOp(Pipe_Op *op) {
	if (op){
		if (op->is_branch){
			printf(
					"OP (PC=%08x inst=%08x) src1=R%d (%08x) src2=R%d (%08x) dst=R%d valid %d (%08x) br=%d taken=%d dest=%08x mem=%d addr=%08x\n",
					op->pc, op->instruction, op->reg_src1, op->reg_src1_value,
					op->reg_src2, op->reg_src2_value, op->reg_dst,
					op->reg_dst_value_ready, op->reg_dst_value, op->is_branch,
					op->branch_taken, op->branch_dest, op->is_mem, op->mem_addr);
		} else {
			printf(
					"OP (PC=%08x inst=%08x) src1=R%d (%08x) src2=R%d (%08x) dst=R%d valid %d (%08x) mem=%d addr=%08x\n",
					op->pc, op->instruction, op->reg_src1, op->reg_src1_value,
					op->reg_src2, op->reg_src2_value, op->reg_dst,
					op->reg_dst_value_ready, op->reg_dst_value, 
					 op->is_mem, op->mem_addr);
		}
	}else{
		printf("(null)\n");
	}
}

PipeState::PipeState() :
		fetch_op(nullptr), decode_op(nullptr), dispatch_op(nullptr), execute_op(nullptr), mem_op(
				nullptr), wb_op(nullptr), data_mem(nullptr), inst_mem(nullptr), HI(
				0), LO(0), branch_recover(0), branch_dest(0), branch_flush(0), RUN_BIT(
				true), stat_cycles(0), stat_inst_retire(0), stat_inst_fetch(0), stat_squash(
				0) {
	//initialize the register file
	for (int i = 0; i < 64; i++) {
		REGS[i] = 0;
	}
	
	//initialize map table
	DPRINTF(DEBUG_PIPE, "Pipe.cpp\n");
	mapTable = new MapTable();
	//initialize Architectural Map Table
	archMapTable = new MapTable();

	//initialize Free List
	//void *c = (FreeList*) malloc (sizeof(int) + sizeof(queue<int>));
	freeList = new FreeList(64); // TODO: make these values based on config value
	freeList -> init(32);
	//initialize LSQ
	lsq = new LSQ();
	//initialize ROB
	rob = new ROB();

	rs = new RS();
	
	//initialize PC
	PC = 0x00400000;
	//initialize the branch predictor
	BP = new StaticNTBranchPredictor();
}

PipeState::PipeState(uint32_t rob_size, uint32_t lsq_size) :
		fetch_op(nullptr), decode_op(nullptr), dispatch_op(nullptr), execute_op(nullptr), mem_op(
				nullptr), wb_op(nullptr), data_mem(nullptr), inst_mem(nullptr), HI(
				0), LO(0), branch_recover(0), branch_dest(0), branch_flush(0), RUN_BIT(
				true), stat_cycles(0), stat_inst_retire(0), stat_inst_fetch(0), stat_squash(
				0) {
	//initialize the register file
	DPRINTF(DEBUG_PIPE, "HELLO\n");
	for (int i = 0; i < 64; i++) {
		REGS[i] = 0;
	}
	DPRINTF(DEBUG_PIPE, "HELLO\n");
	//initialize map table
	mapTable = new MapTable();
	//initialize Architectural Map Table
	archMapTable = new MapTable();
	//initialize Free List
	freeList = new FreeList(64); // TODO make these values based on config value
	freeList -> init(32);
	//initialize LSQ
	lsq = new LSQ(lsq_size);
	//initializing RS
	rs = new RS();
	rob = new ROB();

	//initialize PC
	PC = 0x00400000;
	//initialize the branch predictor
	BP = new StaticNTBranchPredictor();
}

PipeState::~PipeState() {
	if (fetch_op)
		free(fetch_op);
	if (decode_op)
		free(decode_op);
	if (dispatch_op)
		free(dispatch_op);
	if (execute_op)
		free(execute_op);
	if (mem_op)
		free(mem_op);
	if (wb_op)
		free(wb_op);
	delete BP;
}

void PipeState::pipeCycle() {
	if (DEBUG_PIPE) {
		printf("\n\n----\nCycle : %lu\nPIPELINE:\n", currCycle);
	}
	printPipeState();
		/*
		printf("DECODE: ");
		printOp(decode_op);
		printf("DISPATCH: ");
		printOp(dispatch_op);
		printf("EXEC : ");
		printOp(execute_op);
		printf("MEM  : ");
		printOp(mem_op);
		printf("WB   : ");
		printOp(wb_op);*/
	
	//This part should be all set
	pipeStageRetire();
	if(RUN_BIT == false)
    		return;
	pipeStageComplete();
	pipeStageExecute();
	pipeStageIssue();
	pipeStageDispatch();
	pipeStageFetch();
	//handle branch recoveries
	if (branch_recover) {
		//TODO: branch flushing needs to be completely redone
		// TODO: move tail up to branch instruction
		// TODO: restore some data structures (Map Table and RS)
		// TODO:move LSQ tail
		DPRINTF(DEBUG_PIPE, "branch recovery: new dest %08x flush %d stages\n",
				branch_dest, branch_flush);

		PC = branch_dest;

		if (branch_flush >= 1) {
			if (fetch_op)
				free(fetch_op);
			fetch_op = nullptr;
		}

		if (branch_flush >= 2) {
			if (decode_op)
				free(decode_op);
			decode_op = nullptr;
		}
		
		if (branch_flush >= 2) {
			if (dispatch_op)
				free(dispatch_op);
			dispatch_op = nullptr;
		}

		if (branch_flush >= 3) {
			if (execute_op)
				free(execute_op);
			execute_op = nullptr;
		}

		if (branch_flush >= 4) {
			if (mem_op)
				free(mem_op);

			mem_op = nullptr;
		}

		if (branch_flush >= 5) {
			if (wb_op)
				free(wb_op);
			wb_op = nullptr;
		}

		branch_recover = 0;
		branch_dest = 0;
		branch_flush = 0;

		stat_squash++;
	}
}

void PipeState::pipeRecover(Pipe_Op* op) {
	/* if there is already a recovery scheduled, it must have come from a later
	 * stage (which executes older instructions), hence that recovery overrides
	 * our recovery. Simply return in this case. */
	if (branch_recover)
		return;
	//schedule the recovery. This will be done once all pipeline stages simulate the current cycle.
	branch_recover = 1;
	//branch_flush = flush;
	//branch_dest = dest;
}

/*
void PipeState::pipeStageWb() {
	//if there is no instruction in this pipeline stage, we are done
	if (!wb_op)
		return;
	//grab the op out of our input slot
	Pipe_Op *op = wb_op;
	wb_op = NULL;

	//if this instruction writes a register, do so now
	if (op->reg_dst != -1 && op->reg_dst != 0) {
		REGS[mapTable->getPRegId(op->reg_dst)] = op->reg_dst_value;

		DPRINTF(DEBUG_PIPE, "R%d = %08x\n", op->reg_dst, op->reg_dst_value);
	}
	//if this was a syscall, perform action
	if (op->opcode == OP_SPECIAL && op->subop == SUBOP_SYSCALL) {
		if (op->reg_src1_value == 0xA) {
			PC = op->pc; // fetch will do pc += 4, then we stop with correct PC 
			RUN_BIT = false;
		}
	}
	//TODO: set reg value ready in the map table
	//free the op
	free(op);
	stat_inst_retire++;
	//TODO: add Told reg to the free list
	//TODO: replace Told with T in the arch map table
}

void PipeState::pipeStageMem() {
	//grab the op out of our input slot
	Pipe_Op *op = mem_op;

	//if there is no instruction in this pipeline stage, we are done
	if (!op)
		return;
	else {
		if (op->is_mem == false) {
//			DPRINTF(DEBUG_PIPE, "clearing memory stage for instruction %x\n",
//					mem_op->pc);
			mem_op = NULL;
			wb_op = op;
			return;
		}
		if (op->memTried == true) {
			if (op->waitOnPktIssue) {
				op->waitOnPktIssue = !(data_mem->sendReq(op->memPkt));
				return;
			}
			if (op->readyForNextStage == false)
				return;
			else {
//				DPRINTF(DEBUG_PIPE,
//						"clearing memory stage for instruction %x\n",
//						mem_op->pc);
				Pipe_Op* op = mem_op;
				mem_op = NULL;
				wb_op = op;
				return;
			}
		}
	}
	op->readyForNextStage = false;
	op->memTried = true;
	switch (op->opcode) {
	case OP_LW:
	case OP_LH:
	case OP_LHU:
	case OP_LB:
	case OP_LBU: {
		uint8_t* data = new uint8_t[4];
		op->memPkt = new Packet(true, false, PacketTypeLoad,
				(op->mem_addr & ~3), 4, data, currCycle);
		break;
	}
	case OP_SB: {
		uint8_t* data = new uint8_t;
		*data = op->mem_value & 0xFF;
		op->memPkt = new Packet(true, true, PacketTypeStore, (op->mem_addr), 1,
				data, currCycle);
		break;
	}
	case OP_SH: {
		uint16_t* data = new uint16_t;
		*data = op->mem_value & 0xFFFF;
		op->memPkt = new Packet(true, true, PacketTypeStore, (op->mem_addr), 2,
				(uint8_t*) data, currCycle);
		break;
	}

	case OP_SW: {
		uint32_t* data = new uint32_t;
		*data = op->mem_value;
		op->memPkt = new Packet(true, true, PacketTypeStore, (op->mem_addr), 4,
				(uint8_t*) data, currCycle);
		break;
	}
	}
	DPRINTF(DEBUG_PIPE,
			"sending pkt from memory stage: addr = %x, size = %d, type = %d \n",
			op->memPkt->addr, op->memPkt->size, op->memPkt->type);
	op->waitOnPktIssue = !(data_mem->sendReq(op->memPkt));
	return;
}*/

void PipeState::pipeStageRetire()
{
	int count =0;
	vector<Pipe_Op*>::iterator op = retire_ops.begin(); //initialize iterator
	while (count < 4 and op < retire_ops.end() and retire_ops.size()>0){
		//free the op
		if (rob->getHead().pc == (*op)->pc) {
			//add Told reg to the free list
			freeList->returnReg(rob->getHead().T_old);
			//replace Told with T in the arch map table
			archMapTable->swapPReg(rob->getHead().T_old, rob->getHead().T);
			// add lsq stuff here
			if ((*op)->is_mem){
				lsq -> freeHead();
				if(!(*op)->mem_write){
					data_mem->sendReq((*op)->memPkt);
				}
			}
			Pipe_Op* copy_op = (*op);
			// reomve from rob
			assert(rob->robUpdateHead());
			retire_ops.erase(op);
			stat_inst_retire++;
			op++;
			count++;
			//code to make the program terminate
			if (copy_op->opcode == OP_SPECIAL && copy_op->subop == SUBOP_SYSCALL) {
				if (copy_op->reg_src1_value == 0xA) {
					PC = copy_op->pc; // fetch will do pc += 4, then we stop with correct PC 
					RUN_BIT = false;
				}
			}
			
		} else {
			op++;
		}
				
	}
	
}

void PipeState::pipeStageComplete()
{
	int count =0;
	vector<Pipe_Op*>::iterator op = complete_ops.begin(); //initialize iterator
	while (count < 4 and op < complete_ops.end()) {
		//set reg value ready in the map table
		mapTable->setReady((*op)->reg_dst);
		//call update slot to ready that tag (CDB broadcast)
		//make this only happen if it just reached the complete cycle [an op won't see multiple cycles here]
		rs->updateSlot(mapTable->getPRegId((*op)->reg_dst), true);
		retire_ops.push_back((*op));
		complete_ops.erase(op);
		op++;
		count++;
	}
	
}



void PipeState::pipeStageExecute() {

	for (vector<Pipe_Op*>::iterator op = execute_ops.begin(); op < execute_ops.end(); op++){

	
		//if a multiply/divide is in progress, decrement cycles until value is ready
		if ((*op)->stall > 0){
			(*op)->stall--;
			continue;
		}
		//if no op to execute, return
		//if ((*op) == NULL)
		//	continue;

		//execute the op
		switch ((*op)->opcode) {
		case OP_SPECIAL:
			(*op)->reg_dst_value_ready = 1;
			switch ((*op)->subop) {
			case SUBOP_SLL:
				(*op)->reg_dst_value = (*op)->reg_src2_value << (*op)->shamt;
				break;
			case SUBOP_SLLV:
				(*op)->reg_dst_value = (*op)->reg_src2_value << (*op)->reg_src1_value;
				break;
			case SUBOP_SRL:
				(*op)->reg_dst_value = (*op)->reg_src2_value >> (*op)->shamt;
				break;
			case SUBOP_SRLV:
				(*op)->reg_dst_value = (*op)->reg_src2_value >> (*op)->reg_src1_value;
				break;
			case SUBOP_SRA:
				(*op)->reg_dst_value = (int32_t) (*op)->reg_src2_value >> (*op)->shamt;
				break;
			case SUBOP_SRAV:
				(*op)->reg_dst_value = (int32_t) (*op)->reg_src2_value
						>> (*op)->reg_src1_value;
				break;
			case SUBOP_JR:
			case SUBOP_JALR:
				(*op)->reg_dst_value = (*op)->pc + 4;
				(*op)->branch_dest = (*op)->reg_src1_value;
				(*op)->branch_taken = 1;
				break;

			case SUBOP_MULT: {
				/* we set a result value right away; however, we will
				 * model a stall if the program tries to read the value
				 * before it's ready (or overwrite HI/LO). Also, if
				 * another multiply comes down the pipe later, it will
				 * update the values and re-set the stall cycle count
				 * for a new operation.
				 */
				int64_t val = (int64_t) ((int32_t) (*op)->reg_src1_value)
						* (int64_t) ((int32_t) (*op)->reg_src2_value);
				uint64_t uval = (uint64_t) val;
				HI = (uval >> 32) & 0xFFFFFFFF;
				LO = (uval >> 0) & 0xFFFFFFFF;

				//four-cycle multiplier latency
				(*op)->stall = 4;
			}
				break;
			case SUBOP_MULTU: {
				uint64_t val = (uint64_t) (*op)->reg_src1_value
						* (uint64_t) (*op)->reg_src2_value;
				HI = (val >> 32) & 0xFFFFFFFF;
				LO = (val >> 0) & 0xFFFFFFFF;

				//four-cycle multiplier latency
				(*op)->stall = 4;
			}
				break;

			case SUBOP_DIV:
				if ((*op)->reg_src2_value != 0) {

					int32_t val1 = (int32_t) (*op)->reg_src1_value;
					int32_t val2 = (int32_t) (*op)->reg_src2_value;
					int32_t div, mod;

					div = val1 / val2;
					mod = val1 % val2;

					LO = div;
					HI = mod;
				} else {
					//really this would be a div-by-0 exception
					HI = LO = 0;
				}

				//2-cycle divider latency
				(*op)->stall = 32;
				break;

			case SUBOP_DIVU:
				if ((*op)->reg_src2_value != 0) {
					HI = (uint32_t) (*op)->reg_src1_value
							% (uint32_t) (*op)->reg_src2_value;
					LO = (uint32_t) (*op)->reg_src1_value
							/ (uint32_t) (*op)->reg_src2_value;
				} else {
					/* really this would be a div-by-0 exception */
					HI = LO = 0;
				}

				/* 32-cycle divider latency */
				(*op)->stall = 32;
				break;

			case SUBOP_MFHI:
				/* stall until value is ready */
				if ((*op)->stall > 0)
					return;

				(*op)->reg_dst_value = HI;
				break;
			case SUBOP_MTHI:
				//stall to respect WAW dependence
				if ((*op)->stall > 0)
					return;

				HI = (*op)->reg_src1_value;
				break;

			case SUBOP_MFLO:
				//stall until value is ready
				if ((*op)->stall > 0)
					return;

				(*op)->reg_dst_value = LO;
				break;
			case SUBOP_MTLO:
				//stall to respect WAW dependence
				if ((*op)->stall > 0)
					return;

				LO = (*op)->reg_src1_value;
				break;

			case SUBOP_ADD:
			case SUBOP_ADDU:
				(*op)->reg_dst_value = (*op)->reg_src1_value + (*op)->reg_src2_value;
				break;
			case SUBOP_SUB:
			case SUBOP_SUBU:
				(*op)->reg_dst_value = (*op)->reg_src1_value - (*op)->reg_src2_value;
				break;
			case SUBOP_AND:
				(*op)->reg_dst_value = (*op)->reg_src1_value & (*op)->reg_src2_value;
				break;
			case SUBOP_OR:
				(*op)->reg_dst_value = (*op)->reg_src1_value | (*op)->reg_src2_value;
				break;
			case SUBOP_NOR:
				(*op)->reg_dst_value = ~((*op)->reg_src1_value | (*op)->reg_src2_value);
				break;
			case SUBOP_XOR:
				(*op)->reg_dst_value = (*op)->reg_src1_value ^ (*op)->reg_src2_value;
				break;
			case SUBOP_SLT:
				(*op)->reg_dst_value =
						((int32_t) (*op)->reg_src1_value < (int32_t) (*op)->reg_src2_value) ?
								1 : 0;
				break;
			case SUBOP_SLTU:
				(*op)->reg_dst_value =
						((*op)->reg_src1_value < (*op)->reg_src2_value) ? 1 : 0;
				break;
			}
			break;

		case OP_BRSPEC:
			switch ((*op)->subop) {
			case BROP_BLTZ:
			case BROP_BLTZAL:
				if ((int32_t) (*op)->reg_src1_value < 0)
					(*op)->branch_taken = 1;
				break;

			case BROP_BGEZ:
			case BROP_BGEZAL:
				if ((int32_t) (*op)->reg_src1_value >= 0)
					(*op)->branch_taken = 1;
				break;
			}
			break;

		case OP_BEQ:
			if ((*op)->reg_src1_value == (*op)->reg_src2_value)
				(*op)->branch_taken = 1;
			break;

		case OP_BNE:
			if ((*op)->reg_src1_value != (*op)->reg_src2_value)
				(*op)->branch_taken = 1;
			break;

		case OP_BLEZ:
			if ((int32_t) (*op)->reg_src1_value <= 0)
				(*op)->branch_taken = 1;
			break;

		case OP_BGTZ:
			if ((int32_t) (*op)->reg_src1_value > 0)
				(*op)->branch_taken = 1;
			break;

		case OP_ADDI:
		case OP_ADDIU:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value = (*op)->reg_src1_value + (*op)->se_imm16;
			break;
		case OP_SLTI:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value =
					(int32_t) (*op)->reg_src1_value < (int32_t) (*op)->se_imm16 ? 1 : 0;
			break;
		case OP_SLTIU:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value =
					(uint32_t) (*op)->reg_src1_value < (uint32_t) (*op)->se_imm16 ? 1 : 0;
			break;
		case OP_ANDI:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value = (*op)->reg_src1_value & (*op)->imm16;
			break;
		case OP_ORI:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value = (*op)->reg_src1_value | (*op)->imm16;
			break;
		case OP_XORI:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value = (*op)->reg_src1_value ^ (*op)->imm16;
			break;
		case OP_LUI:
			(*op)->reg_dst_value_ready = 1;
			(*op)->reg_dst_value = (*op)->imm16 << 16;
			break;

		case OP_LW:
		case OP_LH:
		case OP_LHU:
		case OP_LB:
		case OP_LBU:
			(*op)->mem_addr = (*op)->reg_src1_value + (*op)->se_imm16;
			//TODO: check if it is in the lsq already
			data_mem->sendReq((*op)->memPkt);
			break;

		case OP_SW:
		case OP_SH:
		case OP_SB:
			(*op)->mem_addr = (*op)->reg_src1_value + (*op)->se_imm16;
			(*op)->mem_value = (*op)->reg_src2_value;
			break;
		}

		//update the branch predictor metadata
		BP->update((*op)->pc, (*op)->branch_taken, (*op)->branch_dest);

		//handle branch recoveries at this point		
		//TODO: move branch functionality to issue stage
		if ((*op)->branch_taken)
			pipeRecover((*op));

		//remove from upstream stage and place in downstream stage
		//TODO: conver to vectors
		if ((*op)->stall == 0){
			complete_ops.push_back(*op);
			execute_ops.erase(op);
		}
	}
}


void PipeState::pipeStageIssue() {
	//iterate through the issue_ops and if both bits are set to ready in RS then read values and move to execute
	for (vector<Pipe_Op*>::iterator iter_op = issue_ops.begin(); iter_op < issue_ops.end(); iter_op++){
		if (rs->isReady((*iter_op) ->FU)){
			// read values from source registers here
			(*iter_op) -> reg_src1_value = REGS[(*iter_op) -> reg_src1];// is this the correct register
			(*iter_op) -> reg_src2_value = REGS[(*iter_op) -> reg_src2];
			//TODO: Set delay for multi cycle instruction here
			//TODO: the execute stage takes care of this

			//move from issue to execute
			execute_ops.push_back((*iter_op));
			issue_ops.erase(iter_op);
		}
	}
	 

}

void PipeState::pipeStageDispatch() {
	//dispatch will first decode each operation then do the dispatch actions
	int count = 0; // count how many instructions pass through in a cycle

	// go through instructions in the ROB to see if they can be dispatched
	for (int i = 0; i < rob->getSize(); i++) {
		// check only non dispatched instructions
		if (!rob->isDispatched(i)) {
			robROW row;
			row = rob->getROBrow(i);
			Pipe_Op* op = NULL;
			vector<Pipe_Op*>::iterator to_remove;
			//TODO find operation by PC in dispatch_ops
			for (vector<Pipe_Op*>::iterator iter_op = dispatch_ops.begin(); iter_op < dispatch_ops.end(); iter_op++){
				if ((*iter_op) -> pc == row.pc) {
					op = (*iter_op);
					to_remove = iter_op;
					break;
				}
			}
			assert(op != NULL);
			int FU = op -> FU;
			// 1 check if there is a place in the RS for the OP
			if (!rs->canAllocate(FU)) {
				// NO space in RS
				continue;
			}

			// 2 check if there is a place ins LSQ for the op
			if (op->is_mem and !lsq->canAllocate()) {
				// LSQ full -> stall
				continue;
			} else if (op->is_mem) {
				//LSQ
				op->mem_addr = op->reg_src1_value + op->se_imm16;
				lsqElt x = { op->pc , op->mem_addr};
				lsq->allocate(&x);
			}

			//RS
			int reg_src1 = mapTable->getPRegId(op->reg_src1);
			int reg_src2 = mapTable->getPRegId(op->reg_src2);
			int freeReg = mapTable->getPRegId(op->reg_dst);
			if (reg_src2 == -1) { // no src2 reg
				FU = rs->assignSlot(FU, freeReg, reg_src1, mapTable->getReady(op->reg_src1), reg_src2, true);
			}
			else {
				FU = rs->assignSlot(FU, freeReg, reg_src1, mapTable->getReady(op->reg_src1), reg_src2, mapTable->getReady(op->reg_src2));
			}
			//set dispatched true in ROB
			rob->setDispatchedTrue(i);

			//grab the op out of our input slot and move to execute
			issue_ops.push_back((*to_remove));
			dispatch_ops.erase(to_remove);
			count++;
		}

	}

	vector<Pipe_Op*>::iterator iter_op = dispatch_ops.begin(); //initialize iterator
	while (count < 4 and iter_op < dispatch_ops.end()){

		//if there is no instruction in this pipeline stage, we are done
		//if (iter_op == NULL)
		//	dispatch_ops.erase(iter_op);
		//if downstream stall, return (and leave any input we had)
		//if (issue_ops.size() >= 4){
		//	iter_op++;
		//	return;
		//}

		// If it is in the rob
		int index = rob->findByPC((*iter_op)->pc);
		if (index != -1){ // 
			iter_op++;
			continue;
		}
		
		// get functional unit from op pipeOp info
		decode(*iter_op);
		int FU;
		if ((*iter_op)->is_mem) {
			if ((*iter_op)->mem_write) {
				FU = ST; //ST = 3
			}
			else {
				FU = LD; //LD = 2
			}
		} else if ((*iter_op)->opcode == 17) { //Floating point instr
			FU = FP1; // FP1 = 4
		}
		else {
			FU = ALU1; //ALU1=0
		}
		(*iter_op) -> FU = FU;
		
		
		if (rob->canAllocate()){
			int freeReg = -1;
			//if inst does write to reg
			if ((*iter_op)->is_mem && (*iter_op)->mem_write) {
				rob->allocate(-1, -1, (*iter_op)->pc);
			} else {
				// Check if there is a free register
				if (freeList->numFree() == 0) {
					//No free registers -> stall
					iter_op++;
					continue;
				}
				int T_old = mapTable->getPRegId((*iter_op)->reg_dst);
				// allocate to ROB
				
				freeReg = freeList->getReg();
				mapTable->addMap((*iter_op)->reg_dst, freeReg);
				rob->allocate(freeReg, T_old, (*iter_op)->pc);
			}
			
			// ROB allocated now allocate LSQ, RS				
			// If any of these 3 fails then we setDispatched to false
			// 1 check if there is a place in the RS for the OP
			if (!rs->canAllocate(FU)) {
				// NO space in RS -> stall
				iter_op++;
				continue;
			}

			// 2 check if there is a place ins LSQ for the op
			if ((*iter_op)->is_mem and !lsq->canAllocate()) {
				// LSQ full -> stall
				iter_op++;
				continue;
			} else if ((*iter_op)->is_mem){

				//LSQ
				(*iter_op)->mem_addr = (*iter_op)->reg_src1_value + (*iter_op)->se_imm16;
				lsqElt x = { (*iter_op)->pc , (*iter_op)->mem_addr};
				lsq->allocate(&x);
			}
			//RS
			int reg_src1 = mapTable->getPRegId((*iter_op)->reg_src1);
			int reg_src2 = mapTable->getPRegId((*iter_op)->reg_src2);
			if (reg_src2 == -1) { // no src2 reg
				FU = rs->assignSlot(FU, freeReg, reg_src1, mapTable->getReady((*iter_op)->reg_src1), reg_src2, true);
			} else {
				FU = rs->assignSlot(FU, freeReg, reg_src1, mapTable->getReady((*iter_op)->reg_src1), reg_src2, mapTable->getReady((*iter_op)->reg_src2));
			}
			//set dispatched true in ROB
			rob->setDispatchedTrue(rob->getSize() - 1);
			(*iter_op) -> FU = FU;

			//grab the op out of our input slot, decode it and move to execute
			issue_ops.push_back((*iter_op));
			dispatch_ops.erase(iter_op);
			count++;
		}
		else {
			// ROb full -> stall
			continue;
		}


	}
	if (execute_ops.size() > 0) {
		rs->freeIssued();
	}
}



void PipeState::decode(Pipe_Op *op) {

	
	//set up info fields (source/dest regs, immediate, jump dest) as necessary
	uint32_t opcode = (op->instruction >> 26) & 0x3F;
	uint32_t rs = (op->instruction >> 21) & 0x1F;
	uint32_t rt = (op->instruction >> 16) & 0x1F;
	uint32_t rd = (op->instruction >> 11) & 0x1F;
	uint32_t shamt = (op->instruction >> 6) & 0x1F;
	uint32_t funct1 = (op->instruction >> 0) & 0x1F;
	uint32_t funct2 = (op->instruction >> 0) & 0x3F;
	uint32_t imm16 = (op->instruction >> 0) & 0xFFFF;
	uint32_t se_imm16 = imm16 | ((imm16 & 0x8000) ? 0xFFFF8000 : 0);
	uint32_t targ = (op->instruction & ((1UL << 26) - 1)) << 2;

	op->opcode = opcode;
	op->imm16 = imm16;
	op->se_imm16 = se_imm16;
	op->shamt = shamt;

	switch (opcode) {
	case OP_SPECIAL:
		/* all "SPECIAL" insts are R-types that use the ALU and both source
		 * regs. Set up source regs and immediate value. */
		op->reg_src1 = rs;
		op->reg_src2 = rt;
		op->reg_dst = rd;
		op->subop = funct2;
		if (funct2 == SUBOP_SYSCALL) {
			op->reg_src1 = 2; // v0
			op->reg_src2 = 3; // v1
		}
		if (funct2 == SUBOP_JR || funct2 == SUBOP_JALR) {
			op->is_branch = 1;
			op->branch_cond = 0;
		}

		break;

	case OP_BRSPEC:
		//branches that have -and-link variants come here
		op->is_branch = 1;
		op->reg_src1 = rs;
		op->reg_src2 = rt;
		op->is_branch = 1;
		op->branch_cond = 1; /* conditional branch */
		op->branch_dest = op->pc + 4 + (se_imm16 << 2);
		op->subop = rt;
		if (rt == BROP_BLTZAL || rt == BROP_BGEZAL) {
			/* link reg */
			op->reg_dst = 31;
			op->reg_dst_value = op->pc + 4;
			op->reg_dst_value_ready = 1;
		}
		break;

	case OP_JAL:
		op->reg_dst = 31;
		op->reg_dst_value = op->pc + 4;
		op->reg_dst_value_ready = 1;
		op->branch_taken = 1;
		//fallthrough
	case OP_J:
		op->is_branch = 1;
		op->branch_cond = 0;
		op->branch_taken = 1;
		op->branch_dest = (op->pc & 0xF0000000) | targ;
		break;

	case OP_BEQ:
	case OP_BNE:
	case OP_BLEZ:
	case OP_BGTZ:
		//ordinary conditional branches (resolved after execute)
		op->is_branch = 1;
		op->branch_cond = 1;
		op->branch_dest = op->pc + 4 + (se_imm16 << 2);
		op->reg_src1 = rs;
		op->reg_src2 = rt;
		break;

	case OP_ADDI:
	case OP_ADDIU:
	case OP_SLTI:
	case OP_SLTIU:
		//I-type ALU ops with sign-extended immediates
		op->reg_src1 = rs;
		op->reg_dst = rt;
		break;

	case OP_ANDI:
	case OP_ORI:
	case OP_XORI:
	case OP_LUI:
		//I-type ALU ops with non-sign-extended immediates
		op->reg_src1 = rs;
		op->reg_dst = rt;
		break;

	case OP_LW:
	case OP_LH:
	case OP_LHU:
	case OP_LB:
	case OP_LBU:
	case OP_SW:
	case OP_SH:
	case OP_SB:
		//memory ops
		op->is_mem = 1;
		op->reg_src1 = rs;
		if (opcode == OP_LW || opcode == OP_LH || opcode == OP_LHU
				|| opcode == OP_LB || opcode == OP_LBU) {
			//load
			op->mem_write = 0;
			op->reg_dst = rt;
		} else {
			//store
			op->mem_write = 1;
			op->reg_src2 = rt;
		}
		break;
	}

	
}

void PipeState::pipeStageFetch() {

	// check all previous ops
	for (vector<Pipe_Op*>::iterator op = fetch_ops.begin(); op < fetch_ops.end(); op++){
		//DPRINTF(DEBUG_FETCH, 
	//issue  more ops until full
		if ((*op)->isFetchIssued == false) {
			//if sending the packet was unsuccessful before, try again
			(*op)->isFetchIssued = inst_mem->sendReq((*op)->instFetchPkt);
			continue;
		}
		if (dispatch_ops.size() < 4) { // there is space to move an op to dispatch -> try to move it
			
			if ((*op)->readyForNextStage == false)
				// op not ready for next stage keep waiting
				continue;
			else {
				//op ready for next stage move to dispatch_op
				dispatch_ops.push_back((*op));
				fetch_ops.erase(op);
				stat_inst_fetch++;
			}
		}
		
	}

	while (fetch_ops.size() < 4){
		Pipe_Op* op = (Pipe_Op *) malloc(sizeof(Pipe_Op));
		memset(op, 0, sizeof(Pipe_Op));

		
		op->reg_src1 = op->reg_src2 = op->reg_dst = -1;
		op->pc = PC;
		uint8_t* data = new uint8_t[4];
		op->instFetchPkt = new Packet(true, false, PacketTypeFetch, PC, 4,
				data, currCycle);
		DPRINTF(DEBUG_PIPE, "sending pkt from fetch stage with addr %x \n: ",
				op->instFetchPkt->addr);
		//try to send the memory request
		op->isFetchIssued = inst_mem->sendReq(op->instFetchPkt);
	
		// add op to fetch_ops
		fetch_ops.push_back(op);

		//get the next instruction to fetch from branch predictor
		uint32_t target = BP->getTarget(PC);
		if (target == -1) {
			PC = PC + 4;
		} else {
			PC = target;
		}
	}
}


void PipeState::printPipeState(){
	//fetch instructions
	if(DEBUG_FETCH){
		DPRINTF(DEBUG_FETCH, "There are %d instructions in the fetch stage\n", fetch_ops.size());
		for (vector<Pipe_Op*>::iterator op = fetch_ops.begin(); op < fetch_ops.end(); op++){
			DPRINTF(DEBUG_FETCH, "Fetch Instruction:");
			printOp(*op);
		}
	}
	//dispatch instructions
	if(DEBUG_DISPATCH){
		DPRINTF(DEBUG_DISPATCH, "There are %d instructions in the dispatch stage\n", dispatch_ops.size());
		for (vector<Pipe_Op*>::iterator op = dispatch_ops.begin(); op < dispatch_ops.end(); op++){
			DPRINTF(DEBUG_DISPATCH, "Dispatch Instruction:");
			printOp(*op);
		}
	}
	//issue instructions
	if (DEBUG_ISSUE){
		DPRINTF(DEBUG_ISSUE, "There are %d instructions in the issue stage\n", issue_ops.size());
		for (vector<Pipe_Op*>::iterator op = issue_ops.begin(); op < issue_ops.end(); op++){
			DPRINTF(DEBUG_ISSUE, "Issue Instruction:");
			printOp(*op);
		}
	}
	//execute instructions
	if (DEBUG_EXECUTE) {	
		DPRINTF(DEBUG_EXECUTE, "There are %d instructions in the execute stage\n", execute_ops.size());
		for (vector<Pipe_Op*>::iterator op = execute_ops.begin(); op < execute_ops.end(); op++){
			DPRINTF(DEBUG_EXECUTE, "Execute Instruction:");
			printOp(*op);
		}
	}
	//complete instructions
	if (DEBUG_COMPLETE) {
		DPRINTF(DEBUG_COMPLETE, "There are %d instructions in the complete stage\n", complete_ops.size());
		for (vector<Pipe_Op*>::iterator op = complete_ops.begin(); op < complete_ops.end(); op++){
			DPRINTF(DEBUG_COMPLETE, "Complete Instruction:");
			printOp(*op);
		}
	}
	if (DEBUG_RETIRE){
		//retire instructions
		DPRINTF(DEBUG_RETIRE, "There are %d instructions in the retire stage\n", retire_ops.size());
		for (vector<Pipe_Op*>::iterator op = retire_ops.begin(); op < retire_ops.end(); op++){
			DPRINTF(DEBUG_RETIRE, "Retire Instruction:");
			printOp(*op);
		}
	}


}

bool PipeState::sendReq(Packet* pkt) {
	assert(false && "Nobody send request to core, Core is the boss :D");
	return true;
}

void PipeState::recvResp(Packet* pkt) {
	DPRINTF(DEBUG_PIPE,
			"core received a response for pkt : addr = %x, type = %d\n",
			pkt->addr, pkt->type);
	switch (pkt->type) {
	case PacketTypeFetch:
		//if the pkt-type is fetch proceed with fetching the instruction
		/*if (fetch_op != nullptr && fetch_op->pc == pkt->addr && pkt->size == 4) {
			fetch_op->instruction = *((uint32_t*) pkt->data);
			fetch_op->readyForNextStage = true;
		}*/
		// find which fetch op it belongs to
		for (vector<Pipe_Op*>::iterator op = fetch_ops.begin(); op < fetch_ops.end(); op++){
			if ((*op)->pc == pkt->addr){
				(*op)->instruction = *((uint32_t*) pkt->data);
				(*op)->readyForNextStage = true;
			}
		}
		break;
	case PacketTypeLoad: {
		//TODO: change this
		//if pkt-type is load proceed with loading the data
		if (((mem_op->mem_addr & ~3) == pkt->addr) && pkt->size == 4) {
			uint32_t val = *((uint32_t*) pkt->data);
			//extract needed value
			mem_op->reg_dst_value_ready = 1;
			if (mem_op->opcode == OP_LW) {
				mem_op->reg_dst_value = val;
			} else if (mem_op->opcode == OP_LH || mem_op->opcode == OP_LHU) {
				if (mem_op->mem_addr & 2)
					val = (val >> 16) & 0xFFFF;
				else
					val = val & 0xFFFF;

				// sign-extend
				if (mem_op->opcode == OP_LH)
					val |= (val & 0x8000) ? 0xFFFF8000 : 0;

				mem_op->reg_dst_value = val;
			} else if (mem_op->opcode == OP_LB || mem_op->opcode == OP_LBU) {
				switch (mem_op->mem_addr & 3) {
				case 0:
					val = val & 0xFF;
					break;
				case 1:
					val = (val >> 8) & 0xFF;
					break;
				case 2:
					val = (val >> 16) & 0xFF;
					break;
				case 3:
					val = (val >> 24) & 0xFF;
					break;
				}

				// sign-extend
				if (mem_op->opcode == OP_LB)
					val |= (val & 0x80) ? 0xFFFFFF80 : 0;

				mem_op->reg_dst_value = val;
			}
			mem_op->readyForNextStage = true;
		}
		break;
	}
	case PacketTypeStore:
		if (mem_op->mem_addr == pkt->addr) {
			mem_op->readyForNextStage = true;
		} else {
			assert(false && "Invalid store response from memory or cache");
		}
		break;
	default:
		assert(false && "Invalid response from memory or cache");
	}
	delete pkt;
}
