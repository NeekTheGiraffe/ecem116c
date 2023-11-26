#include "cache.h"
#include "utils.h"

#include <iomanip>

int main (int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Please enter filename as an argument" << std::endl;
		std::exit(1);
	}

	std::vector<Instruction> instructions = parseTrace(argv[1]);

	Memory mem;
	for (const Instruction& insn : instructions)
		mem.controller(insn);

	Stats stats = mem.stats();

	std::cout << std::setprecision(6);
	std::cout << "(" << stats.l1MissRate << "," << stats.l2MissRate << "," << stats.aat << ")" << std::endl;
	return 0;
}
