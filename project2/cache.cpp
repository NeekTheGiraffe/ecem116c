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
	std::cerr << "Instruction: " << instruction.MemR << " " << instruction.MemW << " " << instruction.adr << " " << instruction.data << std::endl;
	std::cerr << "V Contents: ";
	for (int i = 0; i < VICTIM_SIZE; i++)
		if (m_v[i].valid)
			// std::cerr << "(tag=" << m_v[i].tag << ", lru=" << m_v[i].lruPosition << ") ";
			std::cerr << m_v[i].tag << " ";
	std::cerr << std::endl;
	for (int i = 0; i < L2_CACHE_SETS; i++)
	{
		for (int w = 0; w < L2_CACHE_WAYS; w++)
		{
			CacheBlock& b = m_l2[i * L2_CACHE_WAYS + w];
			if (b.valid)
				std::cerr << i << " " << b.tag << " " << b.lruPosition << " " << b.data << std::endl;
		}
	}

	bool read = instruction.MemR;
	int block = instruction.adr / BLOCK_SIZE, offset = instruction.adr % BLOCK_SIZE;
	if (read)
	{
		uint32_t data;
		if (queryL1(block, data))
			return;

		if (queryV(block, data))
		{
			setL1WithCascade(block, data);
			return;
		}

		if (queryL2(block, data))
		{
			setL1WithCascade(block, data);
			return;
		}

		// Fetch from main memory and set L1
		setL1WithCascade(block, fetchDataFromMemory(block));
	}
	else
	{
		// Write memory
		for (int i = 0; i < BLOCK_SIZE; i++)
			m_mainMemory[block * BLOCK_SIZE + i] = (instruction.data >> (i * 8));

		if (setL1IfPresent(block, instruction.data))
			return;
		if (setVIfPresent(block, instruction.data))
			return;
		setL2IfPresent(block, instruction.data);
	}
}

void Memory::setL1WithCascade(int block, uint32_t data)
{
	std::cerr << "Setting L1: " << block << std::endl;
	int index = block % L1_CACHE_SETS, tag = block / L1_CACHE_SETS;
	if (m_l1[index].valid && m_l1[index].tag != tag)
		setVWithCascade(index + m_l1[index].tag * L1_CACHE_SETS, m_l1[index].data);
	m_l1[index] = { tag, 0, data, true };
}
bool Memory::setL1IfPresent(int block, uint32_t data)
{
	int index = block % L1_CACHE_SETS, tag = block / L1_CACHE_SETS;
	m_l1Queries++;
	if (m_l1[index].valid && m_l1[index].tag == tag)
	{
		m_l1[index].data = data;
		return true;
	}
	else
	{
		m_l1Misses++;
		return false;
	}
}
bool Memory::queryL1(int block, uint32_t& data)
{
	int l1Index = block % L1_CACHE_SETS, l1Tag = block / L1_CACHE_SETS;
	m_l1Queries++;
	if (m_l1[l1Index].valid && m_l1[l1Index].tag == l1Tag)
	{
		data = m_l1[l1Index].data;
		return true;
	}
	else
	{
		m_l1Misses++;
		return false;
	}
}

void Memory::setVWithCascade(int block, uint32_t data)
{
	std::cerr << "Setting V: " << block << std::endl;
	int ejectionIndex = -1;
	bool ejectionIndexValid = true;
	int nValid = 0;
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		if (m_v[i].valid)
		{
			nValid++;
			if (ejectionIndexValid && m_v[i].lruPosition == 0)
				ejectionIndex = i;
		}
		else
		{
			ejectionIndex = i;
			ejectionIndexValid = false;
		}
	}
	if (ejectionIndexValid)
	{
		setL2WithCascade(m_v[ejectionIndex].tag, m_v[ejectionIndex].data);
		for (int i = 0; i < VICTIM_SIZE; i++)
			m_v[i].lruPosition--;
		m_v[ejectionIndex] = { block, VICTIM_SIZE - 1, data, true };
	}
	else
	{
		m_v[ejectionIndex] = { block, nValid, data, true };
	}
}
bool Memory::setVIfPresent(int block, uint32_t data)
{
	// ASSUMES THAT STORES RESET THE LRU
	m_vQueries++;
	int found = -1;
	int nValid = 0;
	for (int i = 0; i < VICTIM_SIZE; i++)
	{
		CacheBlock& b = m_v[i];
		if (!b.valid)
			continue;
		nValid++;
		if (b.tag == block)
		{
			b.data = data;
			found = i;
		}
	}
	if (found == -1)
	{
		m_vMisses++;
		return false;
	}
	for (CacheBlock& b : m_v)
	{
		if (b.valid && b.lruPosition > m_v[found].lruPosition )
			b.lruPosition--;
	}
	m_v[found].lruPosition = nValid;
	return true;
}
bool Memory::queryV(int block, uint32_t& data)
{
	//std::cerr << "Querying for block " << block;
	m_vQueries++;
	int removedLru = -1;
	for (CacheBlock& b : m_v)
	{
		if (b.valid && b.tag == block)
		{
			data = b.data;
			b.valid = false;
			removedLru = b.lruPosition;
			break;
		}
	}
	if (removedLru == -1)
	{
		//std::cerr << "Not found" << std::endl;
		m_vMisses++;
		return false;
	}
	//std::cerr << "Found" << std::endl;
	for (CacheBlock& b : m_v)
	{
		if (b.valid && b.lruPosition > removedLru)
			b.lruPosition--;
	}
	return true;
}

void Memory::setL2WithCascade(int block, uint32_t data)
{
	std::cerr << "Setting " << block << std::endl;
	int index = block % L2_CACHE_SETS, tag = block / L2_CACHE_SETS;
	int ejectionIndex = -1;
	bool ejectionIndexValid = true;
	int nValid = 0;
	for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
	{
		if (m_l2[i].valid)
		{
			nValid++;
			if (ejectionIndexValid && m_l2[i].lruPosition == 0)
				ejectionIndex = i;
		}
		else
		{
			ejectionIndex = i;
			ejectionIndexValid = false;
		}
	}
	if (ejectionIndexValid)
	{
		for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
			m_l2[i].lruPosition--;
		m_l2[ejectionIndex] = { tag, L2_CACHE_WAYS - 1, data, true };
	}
	else
	{
		m_l2[ejectionIndex] = { tag, nValid, data, true};
	}
}
bool Memory::setL2IfPresent(int block, uint32_t data)
{
	// ASSUMES THAT STORES RESET THE LRU
	int index = block % L2_CACHE_SETS, tag = block / L2_CACHE_SETS;
	m_l2Queries++;
	int found = -1;
	int nValid = 0;
	for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
	{
		CacheBlock& b = m_l2[i];
		if (!b.valid)
			continue;
		nValid++;
		if (b.tag == block)
		{
			b.data = data;
			found = i;
		}
	}
	if (found == -1)
	{
		m_l2Misses++;
		return false;
	}
	for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
	{
		CacheBlock& b = m_l2[i];
		if (b.valid && b.lruPosition > m_v[found].lruPosition )
			b.lruPosition--;
	}
	m_v[found].lruPosition = nValid;
	return true;
}
bool Memory::queryL2(int block, uint32_t& data)
{
	int index = block % L2_CACHE_SETS, tag = block / L2_CACHE_SETS;
	m_l2Queries++;
	int removedLru = -1;
	for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
	{
		CacheBlock& b = m_l2[i];
		if (b.valid && b.tag == tag)
		{
			data = b.data;
			b.valid = false;
			removedLru = b.lruPosition;
			break;
		}
	}
	if (removedLru == -1)
	{
		//std::cerr << "Not found" << std::endl;
		m_l2Misses++;
		return false;
	}
	//std::cerr << "Found" << std::endl;
	for (int i = index * L2_CACHE_WAYS; i < (index + 1) * L2_CACHE_WAYS; i++)
	{
		CacheBlock& b = m_l2[i];
		if (b.valid && b.lruPosition > removedLru)
			b.lruPosition--;
	}
	return true;
}

uint32_t Memory::fetchDataFromMemory(int block) const
{
	uint32_t data = 0;
	for (int i = block * BLOCK_SIZE; i < block * (BLOCK_SIZE + 1); i++)
	{
		data <<= 8;
		data += m_mainMemory[i];	
	}
	return data;
}

Stats Memory::stats() const
{
	static const int l1HitTime = 1, vHitTime = 1, l2HitTime = 8, mmHitTime = 100;
	float l1MissRate = (float)m_l1Misses / m_l1Queries;
	float vMissRate = (float)m_vMisses / m_vQueries;
	float l2MissRate = (float)m_l2Misses / m_l2Queries;
	return {
		l1MissRate,
		l2MissRate,
		l1HitTime + l1MissRate * (vHitTime + vMissRate * (l2HitTime + l2MissRate * mmHitTime)), 
	};
}
