#pragma once

#include <string>
#include <vector>

struct Instruction
{
	bool MemR; 
	bool MemW; 
	int adr;
	uint32_t data; 
};

// Read the file and convert it to a series of memory instructions
std::vector<Instruction> parseTrace(const std::string& path);
