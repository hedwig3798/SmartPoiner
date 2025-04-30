#include "pch.h"
#include "SharedPointer.h"
#include "UniquePointer.h"
#include "WeakPointer.h"
#include <iostream>

#define MEM_SIZE 1'048'576'000 //1GB

int main()
{
#ifdef _DEBUG
	std::string path("./CustomAllocatorD.dll");
#else
	std::string path("./CustomAllocator.dll");
#endif
	HMODULE hDLL = ::LoadLibraryA(path.c_str());
	if (hDLL == nullptr)
	{
		std::cout << "Cannot Link DLL\n";
		return 0;
	}
	IAllocator* listAllocator = ((IAllocator * (*)(size_t, unsigned char, size_t))GetProcAddress(hDLL, "CreateListAllocator"))(MEM_SIZE, 0, 16);

	SharedPointer<int> sharedTestA = SharedPointer<int>(1, listAllocator, listAllocator);
	SharedPointer<int> sharedTestB = sharedTestA;
	WeakPointer<int> weakTestC = sharedTestA;

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	*sharedTestB = 2;

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	// sharedTestA.Release();
	// sharedTestB.Release();

	std::cout << *sharedTestA << " " << *sharedTestB << " " << *(weakTestC.Lock()) << "\n";

	UniquePointer<int> uniqueTestA = UniquePointer<int>(1, listAllocator);
	std::cout << *uniqueTestA << "\n";
	UniquePointer<int> uniqueTestB = std::move(uniqueTestA);
	// uniqueTestB = uniqueTestA;
	std::cout << *uniqueTestB << "\n";

	return 0;
}

