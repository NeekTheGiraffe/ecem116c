#include "CPU.h"

#include <fstream>
#include <sstream>

// Helper function to extract a range of bits from a bitset
template <unsigned long L, unsigned long M, unsigned long N>
bitset<M - L> range(bitset<N> b)
{
	static_assert(L <= M && M <= N);
	bitset<M - L> result;
	for (int i = 0; i < M - L; i++)
		result[i] = b[i + L];
	return result;
}

template <unsigned long N>
int signExtend(bitset<N> b)
{
	static_assert(N <= 32);
	return ((int)b.to_ulong() << (32 - N)) >> (32 - N);
}

// Concatenates bitsets. Least significant bits first.
template <unsigned long M, unsigned long N>
bitset<M + N> concat(bitset<M> b1, bitset<N> b2)
{
	bitset<M + N> result;
	for (int i = 0; i < M; i++)
		result[i] = b1[i];
	for (int i = 0; i < N; i++)
		result[i + M] = b2[i];
	return result;
}

template <unsigned long M, unsigned long N, unsigned long O, unsigned long P, unsigned long Q>
bitset<M + N + O + P + Q> concat(bitset<M> b1, bitset<N> b2, bitset<O> b3, bitset<P> b4, bitset<Q> b5)
{
	bitset<M + N + O + P + Q> result;
	for (int i = 0; i < M; i++)
		result[i] = b1[i];
	for (int i = 0; i < N; i++)
		result[i + M] = b2[i];
	for (int i = 0; i < O; i++)
		result[i + M + N] = b3[i];
	for (int i = 0; i < P; i++)
		result[i + M + N + O] = b4[i];
	for (int i = 0; i < Q; i++)
		result[i + M + N + O + P] = b5[i];
	return result;
}

Memory::Memory(const char *filename, unsigned int &maxPc)
{
	ifstream infile(filename); // open the file
	if (!(infile.is_open() && infile.good()))
	{
		cerr << "Error opening file\n";
		exit(1);
	}
	string line;
	int i = 0;
	while (infile)
	{
		infile >> line;
		stringstream line2(line);
		int x;
		line2 >> x;
		this->instructions[i] = bitset<8>(x);
		i++;
	}
	maxPc = i;
	for (int i = 0; i < 4096; i++)
	{
		this->data[i] = 0;
	}
}

bitset<32> Memory::fetchInstruction(unsigned int pc)
{
	return ((((instructions[pc + 3].to_ulong()) << 24)) + ((instructions[pc + 2].to_ulong()) << 16) + ((instructions[pc + 1].to_ulong()) << 8) + (instructions[pc + 0].to_ulong()));
}

int Memory::accessData(unsigned int address, int writeData, bool memRead, bool memWrite)
{
	if (memWrite)
	{
		for (int i = 0; i < 4; i++)
			data[address + i] = writeData << (8 * i);
	}
	if (!memRead)
		return 0;

	int result = 0;
	for (int i = 0; i < 4; i++)
		result += data[address + i].to_ulong() << (8 * i);
	return result;
}

DecodedInstruction decode(bitset<32> instruction)
{
	return DecodedInstruction{
		range<0, 7>(instruction),	// opcode
		range<7, 12>(instruction),	// rd
		range<15, 20>(instruction), // rs1
		range<20, 25>(instruction), // rs2
	};
}

ControlSignals mainController(bitset<7> opcode)
{
	// These values were all determined from looking at the instruction opcodes
	// and the table of controller outputs.
	// This will need to be changed if more instructions are added.
	return ControlSignals{
		opcode[2],																							// jump
		opcode[6] && !opcode[2],																			// branch
		!range<4, 6>(opcode).to_ulong(),																	// memRead
		!range<4, 6>(opcode).to_ulong(),																	// memToReg
		(opcode[4] << 1) | ((opcode[4] & !opcode[5]) | (opcode[6] & !opcode[2])),							// aluOp
		!opcode[6] && opcode[5] && !opcode[4],																// memWrite
		(!opcode[6] && opcode[5] && !opcode[4]) || (!opcode[5] && !opcode[4]) || (!opcode[5] && opcode[4]), // aluSrc
		!((!opcode[6] && opcode[5] && !opcode[4]) || (opcode[6] && !opcode[2])),							// regWrite
	};
}

// Returns generated immediate
int immediateGenerator(bitset<32> instruction)
{
	int iTypeImmediate = signExtend(range<20, 32>(instruction));
	int sTypeImmediate = signExtend(concat(range<7, 12>(instruction), range<25, 32>(instruction)));
	int bTypeImmediate = signExtend(concat(
		bitset<1>(0),
		range<8, 12>(instruction),
		range<25, 31>(instruction),
		range<7, 8>(instruction),
		range<31, 32>(instruction)));
	if (instruction[6] && !instruction[2])
		return bTypeImmediate;
	if (!instruction[6] && instruction[5] && !instruction[4])
		return sTypeImmediate;
	return iTypeImmediate;
}

// Returns ALU opcode
bitset<4> aluControl(bitset<32> instruction, bitset<2> aluOp)
{
	// These values were determined by the table of AluOp, funct3 and funct7 to
	// ALU Opcode. They will have to be changed if more ALU instructions are added.
	bitset<3> funct3 = range<12, 15>(instruction);
	bitset<4> result;
	result[0] = 0;
	result[1] = !aluOp[1] | funct3 == 0b000;
	result[2] = aluOp == 0b01 | (aluOp[1] & (funct3 == 0b100 | (funct3 == 0b000 & instruction[30])));
	result[3] = aluOp == 0b10 & funct3 == 0b101;
	return result;
}

int alu(int input1, int input2, bitset<4> opcode)
{
	if (opcode == 0b0010)
		return input1 + input2;
	if (opcode == 0b0110)
		return input1 - input2;
	if (opcode == 0b0100)
		return input1 ^ input2;
	if (opcode == 0b1000)
		return input1 >> input2;
	if (opcode == 0b0000)
		return input1 & input2;
	return 0;
}
