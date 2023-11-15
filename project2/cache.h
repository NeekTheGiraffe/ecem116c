#pragma once

#include "utils.h"

#include <iostream>
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define VICTIM_SIZE 4
#define L2_CACHE_WAYS 8
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // bytes per block
#define DM 0
#define SA 1

struct CacheBlock
{
	int tag;
	int lruPosition;
	uint32_t data;
	bool valid;
};

struct Stats
{
	float l1MissRate;
	float l2MissRate;
	float aat;
};

class Memory
{
public:
	Memory();
	void controller(const Instruction& instruction);
	Stats stats() const;
private:
	CacheBlock m_l1[L1_CACHE_SETS];
	CacheBlock m_v[VICTIM_SIZE];
	CacheBlock m_l2[L2_CACHE_SETS * L2_CACHE_WAYS];
	uint8_t m_mainMemory[MEM_SIZE];
	int m_l1Queries;
	int m_l1Misses;
	int m_vQueries;
	int m_vMisses;
	int m_l2Queries;
	int m_l2Misses;
};


