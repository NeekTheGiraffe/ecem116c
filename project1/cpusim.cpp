#include "CPU.h"

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

int main(int argc, char *argv[])
{
	/* Each cell should store 1 byte. You can define the memory either dynamically, or define it as a fixed size with size 4KB (i.e., 4096 lines). Each instruction is 32 bits (i.e., 4 lines, saved in little-endian mode).
	Each line in the input file is stored as an unsigned char and is 1 byte (each four lines are one instruction). You need to read the file line by line and store it into the memory. You may need a mechanism to convert these values to bits so that you can read opcodes, operands, etc.
	*/

	if (argc < 2)
	{
		cerr << "No file name entered. Exiting...";
		return 1;
	}

	unsigned int maxPc;
	Memory memory(argv[1], maxPc);
	RegisterFile registers;

	unsigned int pc = 0;

#if 1
	cerr << "addr | " << setw(19) << "| " << "  imm | rd | rs1 | rs2 |  opcode | registers" << endl;
#endif
	for (;;)
	{
		// Fetch
		bitset<32> instruction = memory.fetchInstruction(pc);

		// Decode
		if (instruction == 0)
			break;
		DecodedInstruction decodedInsn = decode(instruction);
		int readData1 = registers.get(decodedInsn.rs1);
		int readData2 = registers.get(decodedInsn.rs2);
		int immediate = immediateGenerator(instruction);
		ControlSignals controlSignals = mainController(decodedInsn.opcode);
		bitset<4> aluOpcode = aluControl(instruction, controlSignals.aluOp);

#if 1
		cerr << setw(4) << pc << " | " << controlSignals.jump << " " << controlSignals.branch << " " << controlSignals.memRead << " "
			<< controlSignals.memToReg << " " << controlSignals.aluOp << " " << controlSignals.memWrite << " "
			<< controlSignals.aluSrc << " " << controlSignals.regWrite << " | " << setw(5) << immediate
			<< " | " << setw(2) << decodedInsn.rd.to_ulong() << " | " << setw(3) << decodedInsn.rs1.to_ulong() << " | " 
			<< setw(3) << decodedInsn.rs2.to_ulong() << " | " << setw(7) << decodedInsn.opcode << " | ";
		for (int i = 0; i < 12; i++)
			cerr << setw(3) << registers.get(i) << " ";
		cerr << endl;
#endif

		// Execution
		int aluInput2 = controlSignals.aluSrc ? immediate : readData2;
		int aluResult = alu(readData1, aluInput2, aluOpcode);
		bool aluSignBit = (aluResult >> 31) & 1;
		cerr << "alu data1=" << readData1 << " data2=" << aluInput2 << " opcode=" << aluOpcode << " result=" << aluResult << endl;

		// Memory
		int memoryReadData = memory.accessData(aluResult, readData2, controlSignals.memRead, controlSignals.memWrite);
		unsigned int nextPcSequential = pc + 4;
		unsigned int nextPcJump = controlSignals.jump ? readData1 + immediate : pc + immediate;

		// Writeback
		int memoryResult = controlSignals.memToReg ? memoryReadData : aluResult;
		int registerWriteData = controlSignals.jump ? nextPcSequential : memoryResult;
		bool shouldJump = controlSignals.jump || (controlSignals.branch && aluSignBit);
		registers.set(decodedInsn.rd, registerWriteData);
		pc = shouldJump ? nextPcJump : nextPcSequential;
	}
	int a0 = registers.get(10);
	int a1 = registers.get(11);
	cout << "(" << a0 << "," << a1 << ")" << endl;

	return 0;
}