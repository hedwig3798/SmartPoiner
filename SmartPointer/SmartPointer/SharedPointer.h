#pragma once
#include "IAllocator.h"
#include "ControlBlock.h"

template<typename T> class WeakPointer;

/// <summary>
/// std::shared_pointer를 모방한 클래스
/// 커스텀 할당자를 사용할 수 있게 바꿈
/// </summary>
template<typename T>
class SharedPointer
{
	template<typename T> friend class WeakPointer;

private:
	// 주소
	T* m_address;
	// 컨트롤 블록
	ControlBlock* m_controlBlock;

	// 각 요소에 대해 다른 할당자를 사용할 수 있다.
	IAllocator* m_adressAllocator;
	IAllocator* m_controlBlockAllocator;
private:

	/// <summary>
	/// 생성자
	/// </summary>
	/// <param name="_adressAllocator">객체 할당자</param>
	/// <param name="_controlBlockAllocator">컨트롤 블록 할당자</param>
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
	/// 생성자
	/// </summary>
	/// <param name="_adressAllocator">객체 할당자</param>
	/// <param name="_controlBlockAllocator">컨트롤 블록 할당자</param>
	explicit SharedPointer(IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
	{
		// 각 할당자 존재 여부에 따라 할당 방식이 다르다.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		(m_controlBlockAllocator != nullptr) ? m_controlBlock = (ControlBlock*)(m_controlBlockAllocator->Allocate(sizeof(ControlBlock))) : m_controlBlock = new ControlBlock();
	};

	/// <summary>
	/// 복사를 통한 값을 초기화하는 생성자
	/// </summary>
	/// <param name="_value">값</param>
	/// <param name="_adressAllocator">객체 할당자</param>
	/// <param name="_controlBlockAllocator">컨트롤 블록 할당자</param>
	explicit SharedPointer(T _value, IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
		: m_adressAllocator(_adressAllocator)
		, m_controlBlockAllocator(_controlBlockAllocator)
	{
		// 각 할당자 존재 여부에 따라 할당 방식이 다르다.
		(m_adressAllocator != nullptr) ? m_address = (T*)(m_adressAllocator->Allocate(sizeof(T))) : m_address = new T();
		(m_controlBlockAllocator != nullptr) ? m_controlBlock = (ControlBlock*)(m_controlBlockAllocator->Allocate(sizeof(ControlBlock))) : m_controlBlock = new ControlBlock();
		// 값 초기화
		*m_address = _value;
	};

	/// <summary>
	/// 해제
	/// </summary>
	void Release()
	{
		// 컨트롤 블록이 있다면
		if (m_controlBlock)
		{
			// 각 블록에 대해 갯수를 하나 뺀다.
			// 순차적인 일관성은 없어도 된다. 원자성만 보장되면 됨
			if (m_controlBlock->m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			{
				// 할당자가 있다면 해제 방법이 다르다.
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
	/// 소멸자
	/// </summary>
	~SharedPointer()
	{
		Release();
	}

	/// <summary>
	/// 복사 생성자
	/// </summary>
	/// <param name="_other">복사 될 객체</param>
	SharedPointer(const SharedPointer& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_adressAllocator(_other.m_adressAllocator)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
	{
		// 레퍼런스 카운트 증가
		if (m_controlBlock)
		{
			m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
		}
	}

	/// <summary>
	/// 이동 생성자
	/// </summary>
	/// <param name="_other">이동 할 객체</param>
	SharedPointer(SharedPointer&& _other) noexcept
		: m_address(_other.m_address)
		, m_controlBlock(_other.m_controlBlock)
		, m_adressAllocator(_other.m_adressAllocator)
		, m_controlBlockAllocator(_other.m_controlBlockAllocator)
	{
		// 이동 할 객체에 대한 소유권 포기
		_other.m_address = nullptr;
		_other.m_controlBlock = nullptr;
		_other.m_adressAllocator = nullptr;
		_other.m_controlBlockAllocator = nullptr;
	}

	/// <summary>
	/// 복사 대입 연산자
	/// </summary>
	/// <param name="_other">복사 될 객체</param>
	/// <returns>복사 된 객체</returns>
	SharedPointer& operator=(const SharedPointer& _other) noexcept
	{
		// 다르다면 복사 시작
		if (this != &_other)
		{
			// 현 객체에 이미 있는 자원 해제
			Release();

			// 객체 데이터 복사
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			// 레퍼런스 카운트
			if (m_controlBlock)
			{
				m_controlBlock->m_refCount.fetch_add(1, std::memory_order_relaxed);
			}
		}
		return *this;
	}

	/// <summary>
	/// 이동 대입 연산자
	/// </summary>
	/// <param name="_other"></param>
	/// <returns></returns>
	SharedPointer& operator=(SharedPointer&& _other) noexcept
	{
		if (this != &_other)
		{
			// 현 객체에 이미 있는 자원 해제
			Release();

			// 데이터 이동
			m_address = _other.m_address;
			m_controlBlock = _other.m_controlBlock;
			m_adressAllocator = _other.m_adressAllocator;
			m_controlBlockAllocator = _other.m_controlBlockAllocator;

			// 현 객체의 소유권 포기
			_other.m_address = nullptr;
			_other.m_controlBlock = nullptr;
			_other.m_adressAllocator = nullptr;
			_other.m_controlBlockAllocator = nullptr;
		}
		return *this;
	}

	/// <summary>
	/// 포인터 연산을 도와주기 위한 연산
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
