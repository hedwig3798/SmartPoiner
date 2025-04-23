#pragma once
#include <atomic>

struct ControlBlock
{
	std::atomic<size_t> m_refCount;
	std::atomic<size_t> m_weakCount;

	ControlBlock()
		: m_refCount(1)
		, m_weakCount(1)
	{

	}
};

