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
	/// 생성자
	/// </summary>
	/// <param name="_adressAllocator">커스텀 할당자</param>
	explicit UniquePointer(IAllocator* _adressAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
	{
		// 할당자가 있다면 할당자를 통한 할당을 한다.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
	};

	/// <summary>
	/// 초기화 생성자
	/// </summary>
	/// <param name="_value">초기화 값</param>
	/// <param name="_adressAllocator">커스텀 할당자</param>
	explicit UniquePointer(T _value, IAllocator* _adressAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
	{
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		*m_address = _value;
	};

	/// <summary>
	/// 해제
	/// </summary>
	void Release()
	{
		(m_adressAllocator != nullptr) ? (void)(m_adressAllocator->Deallocate((void*)m_address)) : delete m_address;
		m_address = nullptr;
	}

	/// <summary>
	/// 소멸자
	/// </summary>
	~UniquePointer()
	{
		Release();
	}

	/// <summary>
	/// 복사 생성자
	/// UniquePointer은 고유해야하므로 복사 방지
	/// </summary>
	/// <param name="_other"></param>
	UniquePointer(const UniquePointer& _other) = delete;

	/// <summary>
	/// 이동 생성자
	/// </summary>
	/// <param name="_other">이동할 값</param>
	UniquePointer(UniquePointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// 데이터 이동 후 소유권 포기
		_other.m_address = nullptr;
		_other.m_adressAllocator = nullptr;
	}

	/// <summary>
	/// 복사 대입 연산자
	/// UniquePointer은 고유해야하므로 복사 방지
	/// </summary>
	/// <param name="_other">복사 될 객체</param>
	/// <returns></returns>
	UniquePointer& operator=(const UniquePointer& _other) = delete;

	/// <summary>
	/// 이동 대입 연산자
	/// </summary>
	/// <param name="_other">이동 할 객체</param>
	/// <returns></returns>
	UniquePointer& operator=(UniquePointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// 현재 있는 정보 초기화
			Release();

			// 데이터 이동 및 권한 포기
			m_address = _other.m_address;
			m_adressAllocator = _other.m_adressAllocator;
			_other.m_address = nullptr;
			_other.m_adressAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// 포인터를 쉽게 접근하게 해주는 연산자
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

