/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include "config_reader.h"
#include "simulator.h"
#include "util.h"

/***************************************************************/
/* Simulator.                                                  */
/***************************************************************/
Simulator * simulator;

/***************************************************************/
/*                                                             */
/* Procedure: writeProgramToMem                                */
/*                                                             */
/* Purpose: write the program to the memory                    */
/*                                                             */
/***************************************************************/

void writeProgramToMem(uint32_t address, uint32_t value) {
	int i;
	for (i = 0; i < MEM_NREGIONS; i++) {
		mem_region_t* region = &(simulator->main_memory->MEM_REGIONS[i]);
		if (address >= region->start
				&& address < (region->start + region->size)) {
			uint32_t offset = address - region->start;
			region->mem[offset + 3] = (value >> 24) & 0xFF;
			region->mem[offset + 2] = (value >> 16) & 0xFF;
			region->mem[offset + 1] = (value >> 8) & 0xFF;
			region->mem[offset + 0] = (value >> 0) & 0xFF;
			return;
		}
	}
}

/***************************************************************/
/*                                                             */
/* Procedure: getMemHrchyInfo                                  */
/*                                                             */
/* Purpose: Read memory hierarchy configs                      */
/*                                                             */
/***************************************************************/
MemHrchyInfo* getMemHrchyInfo(char* config_file) {
	FILE * config;
	int ii;
	char line[100];
	config = fopen(config_file, "r");
	assert(config != NULL && "Could not open config file");
	ii = 0;
	std::string str;
	while (fscanf(config, "%s\n", line) != EOF) {
		str.append(line);
	}
	fclose(config);
	ConfigReader msg;
	msg.setJson(str);
	std::cerr << "Config file is read successfully\n";
	MemHrchyInfo * info = new MemHrchyInfo;

	if(msg.getValue("rob_size") != Json::nullValue)
		info->rob_size = msg.getValue("rob_size").asInt();
	else
		std::cerr << "rob_size is not defined in config.json, using default value : " << info->rob_size << "\n";

	if(msg.getValue("lsq_size") != Json::nullValue)
		info->lsq_size = msg.getValue("lsq_size").asInt();
	else
		std::cerr << "lsq_size is not defined in config.json, using default value : " << info->lsq_size << "\n";



	if(msg.getValue("debugMemory") != Json::nullValue)
		DEBUG_MEMORY = msg.getValue("debugMemory").asBool();
	if(msg.getValue("debugPipe") != Json::nullValue)
		DEBUG_PIPE = msg.getValue("debugPipe").asBool();
	if(msg.getValue("debugFetch") != Json::nullValue)
		DEBUG_FETCH = msg.getValue("debugFetch").asBool();
	if(msg.getValue("debugDispatch") != Json::nullValue)
		DEBUG_DISPATCH = msg.getValue("debugDispatch").asBool();
	if(msg.getValue("debugIssue") != Json::nullValue)
		DEBUG_ISSUE = msg.getValue("debugIssue").asBool();
	if(msg.getValue("debugExecute") != Json::nullValue)
		DEBUG_EXECUTE = msg.getValue("debugExecute").asBool();
	if(msg.getValue("debugComplete") != Json::nullValue)
		DEBUG_COMPLETE = msg.getValue("debugComplete").asBool();
	if(msg.getValue("debugRetire") != Json::nullValue)
		DEBUG_RETIRE = msg.getValue("debugRetire").asBool();

	if(msg.getValue("debugAll") != Json::nullValue && msg.getValue("debugAll").asBool())
		DEBUG_MEMORY = DEBUG_PIPE = DEBUG_FETCH = DEBUG_DISPATCH = DEBUG_ISSUE = DEBUG_EXECUTE = DEBUG_COMPLETE = DEBUG_RETIRE = true;
	if(msg.getValue("traceMemory") != Json::nullValue)
		TRACE_MEMORY = msg.getValue("traceMemory").asBool();

	return info;
}

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
	printf("----------------MIPS ISIM Help-----------------------\n");
	printf("go                     -  run program to completion         \n");
	printf("run n                  -  execute program for n instructions\n");
	printf(
			"registerDump                  -  dump architectural registers      \n");
	printf("memDump low high         -  dump memory from low to high      \n");
	printf("input reg_no reg_value - set GPR reg_no to reg_value  \n");
	printf("?                      -  display this help menu            \n");
	printf("quit                   -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : getCommand                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
bool getCommand() {
	char buffer[20];
	int start, stop, cycles;
	int register_no, register_value;

	printf("SIM> ");

	//End of commands
	if (scanf("%s", buffer) == EOF)
		return false;

	printf("\n");

	switch (buffer[0]) {
	case 'G':
	case 'g':
		simulator->go();
		break;

	case 'M':
	case 'm':
		if (scanf("%i %i", &start, &stop) != 2)
			break;

		simulator->memDump(start, stop);
		break;

	case '?':
		help();
		break;
	case 'Q':
	case 'q':
		printf("Bye.\n");
		return false;

	case 'R':
	case 'r':
		if (buffer[1] == 'd' || buffer[1] == 'D')
			simulator->registerDump();
		else {
			if (scanf("%d", &cycles) != 1)
				break;
			simulator->run(cycles);
		}
		break;

	case 'I':
	case 'i':
		if (scanf("%i %i", &register_no, &register_value) != 2)
			break;

		printf("%i %i\n", register_no, register_value);
		simulator->pipe->REGS[register_no] = register_value;
		break;

	case 'H':
	case 'h':
		if (scanf("%i", &register_value) != 1)
			break;

		simulator->pipe->HI = register_value;
		break;

	case 'L':
	case 'l':
		if (scanf("%i", &register_value) != 1)
			break;

		simulator->pipe->LO = register_value;
		break;

	default:
		printf("Invalid Command\n");
		break;
	}
	return true;
}

/**************************************************************/
/*                                                            */
/* Procedure : loadProgram                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void loadProgram(char *program_filename) {
	FILE * prog;
	int ii, word;

	/* Open program file. */
	prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	/* Read in the program. */
	ii = 0;
	while (fscanf(prog, "%x\n", &word) != EOF) {
		writeProgramToMem(MEM_TEXT_START + ii, word);
		ii += 4;
	}

	fclose(prog);

	printf("Read %d words from program into memory.\n\n", ii / 4);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
	int i;
	for (i = 0; i < num_prog_files; i++) {
		loadProgram(program_filename);
		while (*program_filename++ != '\0')
			;
	}
	simulator->pipe->RUN_BIT = true;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {

	/* Error Checking */
	if (argc < 3) {
		printf(
				"Error: usage: %s <config_file> <program_file_1> <program_file_2> ...\n",
				argv[0]);
		exit(1);
	}
	MemHrchyInfo* info = getMemHrchyInfo(argv[1]);
	printf("Simulator...\n\n");

	simulator = new Simulator(info);
	initialize(argv[2], argc - 2);
	while (getCommand())
		;

	delete (simulator);
	delete (info);
	return 0;
}
