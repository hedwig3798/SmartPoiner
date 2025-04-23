#pragma once
#include "IAllocator.h"
#include "ControlBlock.h"

template<typename T>
class SharedPointer
{
	friend class ISmartPointerManager;

private:
	T* m_address;
	ControlBlock* m_controlBlock;

	IAllocator* m_adressAllocator;
	IAllocator* m_controlBlockAllocator;

private:
	explicit SharedPointer(IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
	{
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		(m_controlBlockAllocator != nullptr) ? m_controlBlock = (ControlBlock*)(m_controlBlockAllocator->Allocate(sizeof(ControlBlock))) : m_controlBlock = new ControlBlock();
	};

public:
	void Release()
	{
		if (m_controlBlock)
		{
			m_controlBlock->m_refCount.fetch_sub(1, std::memory_order_acq_rel);
			if (m_controlBlock->m_refCount == 1)
			{
				(m_adressAllocator != nullptr) ? (void)(m_adressAllocator->Deallocate((void*)m_address)) : delete m_address;
				(m_controlBlockAllocator != nullptr) ? (void)(m_controlBlockAllocator->Deallocate((void*)m_controlBlock)) : delete m_controlBlock;

				m_address = nullptr;
				m_controlBlock = nullptr;
			}
		}

	}

	~SharedPointer()
	{
		Release();
	}

	SharedPointer(const SharedPointer& _other)
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
	{
		if (m_controlBlock)
		{
			m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	SharedPointer(SharedPointer&& _other)
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
	{
		_other.m_address = nullptr;
		_other.m_controlBlock = nullptr;
	}

	SharedPointer& operator=(const SharedPointer& _other)
	{
		if (this != &_other)
		{
			Release();
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			if (m_controlBlock)
			{
				m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
			}
		}
		return *this;
	}

	SharedPointer& operator=(SharedPointer&& _other)
	{
		if (this != &_other)
		{
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			_other.m_address = nullptr;
			_other.m_controlBlock = nullptr;
		}
		return *this;
	}

	T& operator*()
	{
		return *m_address;
	}
	T* operator->()
	{
		return m_address;
	}

	T* GetAddress()
	{
		return m_address;
	}
	T** GetAddressOf()
	{
		return &m_address;
	}
};
