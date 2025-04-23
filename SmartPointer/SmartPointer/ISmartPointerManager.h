#pragma once
#include "SharedPointer.h"

class IAllocator;

class ISmartPointerManager
{
public:

	template<typename T>
	static SharedPointer<T> CreateSharedPointer(IAllocator* _adressAllocator = nullptr, IAllocator* _controlBlockAllocator = nullptr)
	{
		return SharedPointer<T>(_adressAllocator, _controlBlockAllocator);
	}
};

