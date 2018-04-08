#include <iostream>
#include <string>

#include "MemAllocator.cpp"

using namespace talloc;

int main()
{
	int totalSize = (sizeof(size_t) + sizeof(size_t*)) * 1024 + sizeof(size_t*) * 11;
	size_t* buffer = static_cast<size_t*>(malloc(totalSize));

	MemAllocator allocator = MemAllocator(buffer, totalSize);

	size_t* a[1024];
	for (size_t i = 0; i < 1030; i++)
	{
		try
		{
			a[i] = reinterpret_cast<size_t*>(allocator.alloc(sizeof(int)));
			*(a[i]) = i;
		}
		catch (invalid_state_error e)
		{
			std::cout << "invalid state at write " << i << std::endl;
		}
		catch (...)
		{
			std::cout << "you are fucked at write " << i << std::endl;
		}
	}

	for (size_t i = 0; i < 1024; i++)
	{
		try
		{
			if (*a[i] != i)
				std::cout << "fuck " << *a[i] << std::endl;
		}
		catch (invalid_state_error e)
		{
			std::cout << "invalid state at read " << i << std::endl;
		}
		catch (...)
		{
			std::cout << "you are fucked at read " << i << std::endl;
		}
	}

	for (size_t i = 0; i < 1024; i += 2)
	{
		try
		{
			allocator.free(a[i]);
		}
		catch (invalid_state_error e)
		{
			std::cout << "invalid state at free " << i << std::endl;
		}
		catch (...)
		{
			std::cout << "you are fucked at free " << i << std::endl;
		}
	}

	for (size_t i = 1; i < 1024; i += 2)
	{
		try
		{
			if (*a[i] != i)
				std::cout << "fuck " << *a[i] << std::endl;
		}
		catch (invalid_state_error e)
		{
			std::cout << "invalid state at read " << i << std::endl;
		}
		catch (...)
		{
			std::cout << "you are fucked at read " << i << std::endl;
		}
	}

	for (size_t i = 1; i < 1024; i += 2)
	{
		try
		{
			allocator.free(a[i]);
		}
		catch (invalid_state_error e)
		{
			std::cout << "invalid state at free " << i << std::endl;
		}
		catch (...)
		{
			std::cout << "you are fucked at free " << i << std::endl;
		}
	}

	std::cout << "done";
	std::string s;
	std::cin >> s;
}
