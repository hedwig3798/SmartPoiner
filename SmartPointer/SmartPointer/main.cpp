#include "pch.h"
#include "SharedPointer.h"
#include "UniquePointer.h"
#include <iostream>

int main()
{
	SharedPointer<int> sharedTestA = SharedPointer<int>(1);
	SharedPointer<int> sharedTestB = sharedTestA;

	std::cout << *sharedTestA << " " << *sharedTestB << "\n";

	*sharedTestB = 2;

	std::cout << *sharedTestA << " " << *sharedTestB << "\n";

	UniquePointer<int> uniqueTestA = UniquePointer<int>(1);
	std::cout << *uniqueTestA << "\n";
	UniquePointer<int> uniqueTestB = std::move(uniqueTestA);
	// uniqueTestB = uniqueTestA;
	std::cout << *uniqueTestB << "\n";

	return 0;
}

