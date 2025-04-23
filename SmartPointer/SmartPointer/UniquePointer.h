#pragma once
#include "IAllocator.h"

template<typename T>
class UniquePointer
{

private:
	T* m_address;

	IAllocator* m_adressAllocator;

public:
	/// <summary>
	/// ������
	/// </summary>
	/// <param name="_adressAllocator">Ŀ���� �Ҵ���</param>
	explicit UniquePointer(IAllocator* _adressAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
	{
		// �Ҵ��ڰ� �ִٸ� �Ҵ��ڸ� ���� �Ҵ��� �Ѵ�.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
	};

	/// <summary>
	/// �ʱ�ȭ ������
	/// </summary>
	/// <param name="_value">�ʱ�ȭ ��</param>
	/// <param name="_adressAllocator">Ŀ���� �Ҵ���</param>
	explicit UniquePointer(T _value, IAllocator* _adressAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
	{
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		*m_address = _value;
	};

	/// <summary>
	/// ����
	/// </summary>
	void Release()
	{
		(m_adressAllocator != nullptr) ? (void)(m_adressAllocator->Deallocate((void*)m_address)) : delete m_address;
		m_address = nullptr;
	}

	/// <summary>
	/// �Ҹ���
	/// </summary>
	~UniquePointer()
	{
		Release();
	}

	/// <summary>
	/// ���� ������
	/// UniquePointer�� �����ؾ��ϹǷ� ���� ����
	/// </summary>
	/// <param name="_other"></param>
	UniquePointer(const UniquePointer& _other) = delete;

	/// <summary>
	/// �̵� ������
	/// </summary>
	/// <param name="_other">�̵��� ��</param>
	UniquePointer(UniquePointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// ������ �̵� �� ������ ����
		_other.m_address = nullptr;
		_other.m_adressAllocator = nullptr;
	}

	/// <summary>
	/// ���� ���� ������
	/// UniquePointer�� �����ؾ��ϹǷ� ���� ����
	/// </summary>
	/// <param name="_other">���� �� ��ü</param>
	/// <returns></returns>
	UniquePointer& operator=(const UniquePointer& _other) = delete;

	/// <summary>
	/// �̵� ���� ������
	/// </summary>
	/// <param name="_other">�̵� �� ��ü</param>
	/// <returns></returns>
	UniquePointer& operator=(UniquePointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// ���� �ִ� ���� �ʱ�ȭ
			Release();

			// ������ �̵� �� ���� ����
			m_address = _other.m_address;
			m_adressAllocator = _other.m_adressAllocator;
			_other.m_address = nullptr;
			_other.m_adressAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// �����͸� ���� �����ϰ� ���ִ� ������
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

