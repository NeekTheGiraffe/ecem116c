#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <array>
using namespace std;

struct Memory
{
	bitset<8> data[4096];
	bitset<8> instructions[4096];

	Memory(const char *filename, unsigned int &maxPc);
	bitset<32> fetchInstruction(unsigned int pc);
	int accessData(unsigned int address, int writeData, bool memRead, bool memWrite);
};

class RegisterFile
{
public:
	RegisterFile() { registers.fill(0); }
	void set(bitset<5> i, int value) {
		if (i != 0)
			registers[i.to_ulong()] = value;
	}
	int get(bitset<5> i) {
		return registers[i.to_ulong()];
	}
private:
	array<int, 32> registers;
};

struct DecodedInstruction
{
	bitset<7> opcode;
	bitset<5> rd;
	bitset<5> rs1;
	bitset<5> rs2;
};

struct ControlSignals
{
	bool jump;
	bool branch;
	bool memRead;
	bool memToReg;
	bitset<2> aluOp;
	bool memWrite;
	bool aluSrc;
	bool regWrite;
};

DecodedInstruction decode(bitset<32> instruction);

ControlSignals mainController(bitset<7> opcode);

// Returns generated immediate
int immediateGenerator(bitset<32> instruction);

// Returns ALU opcode
bitset<4> aluControl(bitset<32> instruction, bitset<2> aluOp);

int alu(int input1, int input2, bitset<4> opcode);
