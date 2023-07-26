#pragma once

#include <vector>

typedef unsigned int uint32;

class StatelessGrouping
{
public:
	void SetSeed(uint32 seed)
	{
		m_seed = seed;
	}

	void SetIndexBits(uint32 bits)
	{
		m_halfIndexBits = bits / 2;
		m_halfIndexBitsMask = (1 << m_halfIndexBits) - 1;
	}

	void SetRoundCount(uint32 count)
	{
		m_roundCount = count;
	}

	void SetGroupSize(uint32 size)
	{
		m_groupSize = size;
	}

	// Get a specific member of a group
	uint32 GetMember(uint32 index, uint32 memberIndex)
	{
		// Bijection function
		index = Encrypt(index);

		// Involution
		index = Involution(index, memberIndex);

		// Inverse bijection function
		return Decrypt(index);
	}

	// Get all members of a group
	void GetGroup(uint32 index, std::vector<uint32>& members)
	{
		// Bijection function
		index = Encrypt(index);

		// Get all the members
		members.resize(m_groupSize);
		for (uint32 i = 0; i < m_groupSize; ++i)
			members[i] = Decrypt(Involution(index, i));
	}

private:
	// Nth order involution, where N = m_groupSize
	uint32 Involution(uint32 x, uint32 offset)
	{
		return m_groupSize * (x / m_groupSize) + ((x + offset) % m_groupSize);
	}

	uint32 Encrypt(uint32 index)
	{
		uint32 left = (index >> m_halfIndexBits);
		uint32 right = index & m_halfIndexBitsMask;

		for (uint32 i = 0; i < m_roundCount; ++i)
		{
			uint32 newLeft = right;
			uint32 newRight = left ^ RoundFunction(right);
			left = newLeft;
			right = newRight;
		}

		return (left << m_halfIndexBits) | right;
	}

	uint32 Decrypt(uint32 index)
	{
		uint32 left = (index >> m_halfIndexBits);
		uint32 right = index & m_halfIndexBitsMask;

		for (uint32 i = 0; i < m_roundCount; ++i)
		{
			uint32 newRight = left;
			uint32 newLeft = right ^ RoundFunction(left);
			left = newLeft;
			right = newRight;
		}

		return (left << m_halfIndexBits) | right;
	}

	uint32 RoundFunction(uint32 x)
	{
		// This is a function of both the input, as well as the key
		return (pcg_hash(x ^ m_seed)) & m_halfIndexBitsMask;
	}

	// https://www.pcg-random.org/
	// https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
	uint32 pcg_hash(uint32 input)
	{
		uint32 state = input * 747796405u + 2891336453u;
		uint32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

private:
	uint32 m_roundCount = 0;
	uint32 m_groupSize = 0;
	uint32 m_halfIndexBits = 0;
	uint32 m_halfIndexBitsMask = 0;
	uint32 m_seed = 0;  // The "random seed" that determines ordering
};
