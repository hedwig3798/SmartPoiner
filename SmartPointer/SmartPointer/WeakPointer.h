#pragma once
#include "IAllocator.h"
#include "ControlBlock.h"
#include "SharedPointer.h"

template<typename T>
class WeakPointer
{
	template<typename T> friend class SharedPointer;

private:
	// �ּ�
	T* m_address;

	// ��Ʈ�� ���
	ControlBlock* m_controlBlock;

	// �� ��ҿ� ���� �ٸ� �Ҵ���
	IAllocator* m_adressAllocator;
	IAllocator* m_controlBlockAllocator;

public:
	/// <summary>
	/// ������
	/// SharedPointer�� �־�� �Ѵ�
	/// </summary>
	/// <param name="_sharedPointer">Shared Pointer</param>
	WeakPointer(SharedPointer<T>& _sharedPointer)
		: m_address(_sharedPointer.m_address)
		, m_controlBlock(_sharedPointer.m_controlBlock)
		, m_adressAllocator(_sharedPointer.m_adressAllocator)
		, m_controlBlockAllocator(_sharedPointer.m_controlBlockAllocator)
	{
		m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
	};

	/// <summary>
	/// ����
	/// </summary>
	void Release()
	{
		// ��Ʈ�� ����� �ִٸ�
		if (m_controlBlock)
		{
			if (m_controlBlock->m_weakCount.fetch_sub(1, std::memory_order_acq_rel) == 1 &&
				m_controlBlock->m_refCount.load(std::memory_order_relaxed) == 0)
			{
				(m_controlBlockAllocator != nullptr) ? (void)(m_controlBlockAllocator->Deallocate((void*)m_controlBlock)) : delete m_controlBlock;
			}
			m_address = nullptr;
			m_controlBlock = nullptr;
		}
	}

	/// <summary>
	/// �Ҹ���
	/// </summary>
	~WeakPointer()
	{
		Release();
	}

	/// <summary>
	/// ���� ������
	/// </summary>
	/// <param name="_other">���� �� ��ü</param>
	WeakPointer(const WeakPointer& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// ���۷��� ī��Ʈ ����
		if (m_controlBlock)
		{
			m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	/// <summary>
	/// �̵� ������
	/// </summary>
	/// <param name="_other">�̵� �� ��ü</param>
	WeakPointer(WeakPointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// �̵� �� ��ü�� ���� ������ ����
		_other.m_address = nullptr;
		_other.m_controlBlock = nullptr;
		_other.m_controlBlockAllocator = nullptr;
		_other.m_adressAllocator = nullptr;
	}

	/// <summary>
	/// ���� ���� ������
	/// </summary>
	/// <param name="_other">���� �� ��ü</param>
	/// <returns>���� �� ��ü</returns>
	WeakPointer& operator=(const WeakPointer& _other) noexcept
	{
		// �ٸ��ٸ� ���� ����
		if (this != &_other)
		{
			// �� ��ü�� �̹� �ִ� �ڿ� ����
			Release();

			// ��ü ������ ����
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;
			m_adressAllocator = _other.m_adressAllocator;

			// ���۷��� ī��Ʈ
			if (m_controlBlock)
			{
				m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
			}
		}
		return *this;
	}

	/// <summary>
	/// �̵� ���� ������
	/// </summary>
	/// <param name="_other"></param>
	/// <returns></returns>
	WeakPointer& operator=(WeakPointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// �� ��ü�� �̹� �ִ� �ڿ� ����
			Release();

			// ������ �̵�
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;
			m_adressAllocator = _other.m_adressAllocator;

			// �� ��ü�� ������ ����
			_other.m_address = nullptr;
			_other.m_controlBlock = nullptr;
			_other.m_controlBlockAllocator = nullptr;
			_other.m_adressAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// ������ ������ �����ֱ� ���� ����
	/// </summary>
	inline bool Expired() const noexcept
	{
		return !m_controlBlock || m_controlBlock->m_refCount.load(std::memory_order_relaxed) == 0;
	}

	inline SharedPointer<T> Lock() const noexcept
	{
		if (Expired())
		{
			return SharedPointer<T>();
		}
		else
		{
			return SharedPointer<T>(m_adressAllocator, m_controlBlockAllocator, m_address, m_controlBlock);
		}
	}
};


