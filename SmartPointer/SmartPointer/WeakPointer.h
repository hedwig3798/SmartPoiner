#pragma once
#include "IAllocator.h"
#include "ControlBlock.h"
#include "SharedPointer.h"

template<typename T>
class WeakPointer
{
	template<typename T> friend class SharedPointer;

private:
	// 주소
	T* m_address;

	// 컨트롤 블록
	ControlBlock* m_controlBlock;

	// 각 요소에 대한 다른 할당자
	IAllocator* m_adressAllocator;
	IAllocator* m_controlBlockAllocator;

public:
	/// <summary>
	/// 생성자
	/// SharedPointer가 있어야 한다
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
	/// 해제
	/// </summary>
	void Release()
	{
		// 컨트롤 블록이 있다면
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
	/// 소멸자
	/// </summary>
	~WeakPointer()
	{
		Release();
	}

	/// <summary>
	/// 복사 생성자
	/// </summary>
	/// <param name="_other">복사 될 객체</param>
	WeakPointer(const WeakPointer& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// 레퍼런스 카운트 증가
		if (m_controlBlock)
		{
			m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	/// <summary>
	/// 이동 생성자
	/// </summary>
	/// <param name="_other">이동 할 객체</param>
	WeakPointer(WeakPointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
		, m_adressAllocator(_other.m_adressAllocator)
	{
		// 이동 할 객체에 대한 소유권 포기
		_other.m_address = nullptr;
		_other.m_controlBlock = nullptr;
		_other.m_controlBlockAllocator = nullptr;
		_other.m_adressAllocator = nullptr;
	}

	/// <summary>
	/// 복사 대입 연산자
	/// </summary>
	/// <param name="_other">복사 될 객체</param>
	/// <returns>복사 된 객체</returns>
	WeakPointer& operator=(const WeakPointer& _other) noexcept
	{
		// 다르다면 복사 시작
		if (this != &_other)
		{
			// 현 객체에 이미 있는 자원 해제
			Release();

			// 객체 데이터 복사
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;
			m_adressAllocator = _other.m_adressAllocator;

			// 레퍼런스 카운트
			if (m_controlBlock)
			{
				m_controlBlock->m_weakCount.fetch_add(1, std::memory_order_relaxed);
			}
		}
		return *this;
	}

	/// <summary>
	/// 이동 대입 연산자
	/// </summary>
	/// <param name="_other"></param>
	/// <returns></returns>
	WeakPointer& operator=(WeakPointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// 현 객체에 이미 있는 자원 해제
			Release();

			// 데이터 이동
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;
			m_adressAllocator = _other.m_adressAllocator;

			// 현 객체의 소유권 포기
			_other.m_address = nullptr;
			_other.m_controlBlock = nullptr;
			_other.m_controlBlockAllocator = nullptr;
			_other.m_adressAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// 포인터 연산을 도와주기 위한 연산
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


