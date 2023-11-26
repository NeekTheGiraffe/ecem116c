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
	// Utilities for bringing memory blocks to L1 and
	// sequentially evicting
	void setL1WithCascade(int block, uint32_t data);
	void setVWithCascade(int block, uint32_t data);
	void setL2WithCascade(int block, uint32_t data);
	// Utilities for SW instructions
	bool setL1IfPresent(int block, uint32_t data);
	bool setVIfPresent(int block, uint32_t data);
	bool setL2IfPresent(int block, uint32_t data);
	// Look for block in caches and update stats accordingly
	bool queryL1(int block, uint32_t& data);
	bool queryV(int block, uint32_t& data);
	bool queryL2(int block, uint32_t& data);

	uint32_t fetchDataFromMemory(int block) const;

	CacheBlock m_l1[L1_CACHE_SETS];
	// Invariant:
	// lru values in valid entries range from 0 to # of valid entries - 1
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


