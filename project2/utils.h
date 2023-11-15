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

std::vector<Instruction> parseTrace(const std::string& path);
