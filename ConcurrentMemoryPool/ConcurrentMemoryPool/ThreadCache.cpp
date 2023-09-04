#include "ThreadCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ...
	return nullptr;
}

void* ThreadCache::Allocate(size_t size) // 分配
{
	assert(size <= MAX_BYTES); // 超过最大分配的内存
	size_t alignSize = SizeClass::RoundUp(size); //对齐大小
	size_t index = SizeClass::Index(size); // 哈希桶索引

	if (!_freeLists[index].Empty())
	{
		return _freeLists[index].Pop();
	}
	else
	{
		return FetchFromCentralCache(index, alignSize);
	}
}

void ThreadCache::Deallocate(void* ptr, size_t size) 
{
	assert(ptr);
	assert(size <= MAX_BYTES);

	// 找对映射的自由链表桶，对象插入进入
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}