#include "cache.h"

#include <algorithm>

Memory::Memory()
	: m_l1Queries(0), m_l1Misses(0),
	  m_vQueries(0), m_vMisses(0),
	  m_l2Queries(0), m_l2Misses(0)
{
	std::fill(m_l1, m_l1 + L1_CACHE_SETS, CacheBlock{ 0, 0, 0, false });
	std::fill(m_v, m_v + VICTIM_SIZE, CacheBlock{ 0, 0, 0, false });
	std::fill(m_l2, m_l2 + L2_CACHE_SETS * L2_CACHE_WAYS, CacheBlock{ 0, 0, 0, false });
	std::fill(m_mainMemory, m_mainMemory + MEM_SIZE, 0);
}

void Memory::controller(const Instruction& instruction)
{
	//std::cerr << instruction.MemR << " " << instruction.MemW << " " << instruction.adr << " " << instruction.data << std::endl;

	bool read = instruction.MemR;
	int block = instruction.adr / BLOCK_SIZE, offset = instruction.adr % BLOCK_SIZE;
	if (read)
	{
		// Check L1
		int l1Index = block % L1_CACHE_SETS, l1Tag = block / L1_CACHE_SETS;
		m_l1Queries++;
		if (m_l1[l1Index].valid && m_l1[l1Index].tag == l1Tag)
			return;
		m_l1Misses++;
		//std::cerr << "L1 miss" << std::endl;

		// Check victim
		/*
		m_vQueries++;
		for (CacheBlock& b : m_v)
		{
			if (b.valid && b.tag == block)
			{
				// TODO: Evict from L1 and update LRU
				m_l1[l1Index] = b;
				return;
			}
		}

		// Check L2
		int l2Index = block % L2_CACHE_SETS, l2Tag = block / L2_CACHE_SETS;
		for (int i = 0; i < L2_CACHE_WAYS; i++)
		{
			CacheBlock& b = m_l2[l2Index * L2_CACHE_WAYS + i];
		}
		*/

		// Fetch from main memory and set L1
		uint32_t data = 0;
		for (int i = block * BLOCK_SIZE; i < block * (BLOCK_SIZE + 1); i++)
		{
			data <<= 8;
			data += m_mainMemory[i];	
		}
		// std::cerr << "my tag " << l1Tag << std::endl;
		// std::cerr << "old tag" << m_l1[l1Index].tag << std::endl;
		m_l1[l1Index] = { l1Tag, 0, data, true };
		// std::cerr << "new tag" << m_l1[l1Index].tag << std::endl;
	}
	else
	{
		// Write memory
		for (int i = 0; i < BLOCK_SIZE; i++)
			m_mainMemory[block * BLOCK_SIZE + i] = (instruction.data >> (i * 8));

		int l1Index = block % L1_CACHE_SETS, l1Tag = block / L1_CACHE_SETS;
		m_l1Queries++;
		if (m_l1[l1Index].valid && m_l1[l1Index].tag == l1Tag)
		{
			m_l1[l1Index].data = instruction.data;
			return;
		}
		m_l1Misses++;
		//std::cerr << "L1 miss" << std::endl;
	}
}

Stats Memory::stats() const
{
	static const int l1HitTime = 1, vHitTime = 1, l2HitTime = 8, mmHitTime = 100;
	float l1MissRate = (float)m_l1Misses / m_l1Queries;
	//float vMissRate = (float)m_vMisses / m_vQueries;
	//float l2MissRate = (float)m_l2Misses / m_l2Queries;
	float vMissRate = 1;
	float l2MissRate = 1;
	//std::cerr << "L1 misses " << m_l1Misses << " / " << m_l1Queries << std::endl;
	return {
		l1MissRate,
		l2MissRate,
		l1HitTime + l1MissRate * (vHitTime + vMissRate * (l2HitTime + l2MissRate * mmHitTime)), 
	};
}
