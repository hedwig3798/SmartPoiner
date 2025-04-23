#include "pch.h"
#include "SharedPointer.h"
#include "UniquePointer.h"
#include "WeakPointer.h"
#include <iostream>

int main()
{
	SharedPointer<int> sharedTestA = SharedPointer<int>(1);
	SharedPointer<int> sharedTestB = sharedTestA;
	WeakPointer<int> weakTestC = sharedTestA;

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	*sharedTestB = 2;

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	// sharedTestA.Release();
	// sharedTestB.Release();

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	UniquePointer<int> uniqueTestA = UniquePointer<int>(1);
	std::cout << *uniqueTestA << "\n";
	UniquePointer<int> uniqueTestB = std::move(uniqueTestA);
	// uniqueTestB = uniqueTestA;
	std::cout << *uniqueTestB << "\n";

	return 0;
}

