#include "pch.h"
#include "ISmartPointerManager.h"
#include <iostream>

int main()
{
	SharedPointer<int> a = ISmartPointerManager::CreateSharedPointer<int>();
	SharedPointer<int> b = a;

	*a = 1;

	std::cout << *a << " " << *b << "\n";

	*b = 2;

	std::cout << *a << " " << *b << "\n";

	return 0;
}

