#pragma once
#include <cassert>

class StackAllocator
{
public:
	StackAllocator(size_t capacity=65536);
	template<typename T>
	T* New()
	{
		// Someone asks for a new object of type T
		// We're not gonna worry about the constructor for now

		size_t size = sizeof(T);
		size_t spaceSoFar = head - buffer;
		if (spaceSoFar + size > capacity)
		{
			// Can't allocate :(
			// Could instead crash the program here, if preferred:
			//     System::MessageBox("Stack allocator ran out of memory!");
			//     exit(1);
			
			// assert(false);
			return nullptr;
		}

		char* oldHead = head;
		head += size;
		return (T*)oldHead;
	}

	template <typename T>
	T* New(unsigned int count)
	{
		// Allocates "count" objects of type T and
		// return a pointer to the first one.
		return nullptr;
	}

	void Clear();
	// void PopTo(void* object);

private:
	char* buffer;
	size_t capacity;
	char* head;
};

StackAllocator::StackAllocator(size_t capacity) :
	buffer(new char[capacity]),
	capacity(capacity),
	head(buffer)
{}

void StackAllocator::Clear()
{
	head = buffer;
}