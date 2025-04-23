#pragma once
#include "IAllocator.h"
#include "ControlBlock.h"

template<typename T> class WeakPointer;

/// <summary>
/// std::shared_pointer�� ����� Ŭ����
/// Ŀ���� �Ҵ��ڸ� ����� �� �ְ� �ٲ�
/// </summary>
template<typename T>
class SharedPointer
{
	template<typename T> friend class WeakPointer;

private:
	// �ּ�
	T* m_address;
	// ��Ʈ�� ���
	ControlBlock* m_controlBlock;

	// �� ��ҿ� ���� �ٸ� �Ҵ��ڸ� ����� �� �ִ�.
	IAllocator* m_adressAllocator;
	IAllocator* m_controlBlockAllocator;
private:

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="_adressAllocator">��ü �Ҵ���</param>
	/// <param name="_controlBlockAllocator">��Ʈ�� ��� �Ҵ���</param>
	explicit SharedPointer(IAllocator* _adressAllocator, IAllocator* _controlBlockAllocator, T* _address, ControlBlock* _controlBlock)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
		, m_address(_address)
		, m_controlBlock(_controlBlock)
	{
		m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
	};

public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="_adressAllocator">��ü �Ҵ���</param>
	/// <param name="_controlBlockAllocator">��Ʈ�� ��� �Ҵ���</param>
	explicit SharedPointer(IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
	{
		// �� �Ҵ��� ���� ���ο� ���� �Ҵ� ����� �ٸ���.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		(m_controlBlockAllocator != nullptr) ? m_controlBlock = (ControlBlock*)(m_controlBlockAllocator->Allocate(sizeof(ControlBlock))) : m_controlBlock = new ControlBlock();
	};

	/// <summary>
	/// ���縦 ���� ���� �ʱ�ȭ�ϴ� ������
	/// </summary>
	/// <param name="_value">��</param>
	/// <param name="_adressAllocator">��ü �Ҵ���</param>
	/// <param name="_controlBlockAllocator">��Ʈ�� ��� �Ҵ���</param>
	explicit SharedPointer(T _value, IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
	{
		// �� �Ҵ��� ���� ���ο� ���� �Ҵ� ����� �ٸ���.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		(m_controlBlockAllocator != nullptr) ? m_controlBlock = (ControlBlock*)(m_controlBlockAllocator->Allocate(sizeof(ControlBlock))) : m_controlBlock = new ControlBlock();
		// �� �ʱ�ȭ
		*m_address = _value;
	};

	/// <summary>
	/// ����
	/// </summary>
	void Release()
	{
		// ��Ʈ�� ����� �ִٸ�
		if (m_controlBlock)
		{
			// �� ��Ͽ� ���� ������ �ϳ� ����.
			// �������� �ϰ����� ��� �ȴ�. ���ڼ��� ����Ǹ� ��
			if (m_controlBlock->m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				// �Ҵ��ڰ� �ִٸ� ���� ����� �ٸ���.
				(m_adressAllocator != nullptr) ? (void)(m_adressAllocator->Deallocate((void*)m_address)) : delete m_address;
				m_address = nullptr;

				if (m_controlBlock->m_weakCount.load(std::memory_order_relaxed) == 0)
				{
					(m_controlBlockAllocator != nullptr) ? (void)(m_controlBlockAllocator->Deallocate((void*)m_controlBlock)) : delete m_controlBlock;
					m_controlBlock = nullptr;
				}
			}
		}

	}

	/// <summary>
	/// �Ҹ���
	/// </summary>
	~SharedPointer()
	{
		Release();
	}

	/// <summary>
	/// ���� ������
	/// </summary>
	/// <param name="_other">���� �� ��ü</param>
	SharedPointer(const SharedPointer& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_adressAllocator(_other.m_adressAllocator)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
	{
		// ���۷��� ī��Ʈ ����
		if (m_controlBlock)
		{
			m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	/// <summary>
	/// �̵� ������
	/// </summary>
	/// <param name="_other">�̵� �� ��ü</param>
	SharedPointer(SharedPointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_adressAllocator(_other.m_adressAllocator)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
	{
		// �̵� �� ��ü�� ���� ������ ����
		_other.m_address = nullptr;
		_other.m_controlBlock = nullptr;
		_other.m_adressAllocator = nullptr;
		_other.m_controlBlockAllocator = nullptr;
	}

	/// <summary>
	/// ���� ���� ������
	/// </summary>
	/// <param name="_other">���� �� ��ü</param>
	/// <returns>���� �� ��ü</returns>
	SharedPointer& operator=(const SharedPointer& _other) noexcept
	{
		// �ٸ��ٸ� ���� ����
		if (this != &_other)
		{
			// �� ��ü�� �̹� �ִ� �ڿ� ����
			Release();

			// ��ü ������ ����
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			// ���۷��� ī��Ʈ
			if (m_controlBlock)
			{
				m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
			}
		}
		return *this;
	}

	/// <summary>
	/// �̵� ���� ������
	/// </summary>
	/// <param name="_other"></param>
	/// <returns></returns>
	SharedPointer& operator=(SharedPointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// �� ��ü�� �̹� �ִ� �ڿ� ����
			Release();

			// ������ �̵�
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_adressAllocator = _other.m_adressAllocator;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;

			// �� ��ü�� ������ ����
			_other.m_address = nullptr;
			_other.m_controlBlock = nullptr;
			_other.m_adressAllocator = nullptr;
			_other.m_controlBlockAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// ������ ������ �����ֱ� ���� ����
	/// </summary>
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
};
