#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// 慢开始反馈调节算法
	// 1. 最开始不会一次向CentralCache一次批量要太多, 因为会造成浪费
	// 2. 如果你不断有这个size大小内存需求, 那么batchNum就会不断增长, 直到上限512
	// 3. size越大, 一次向CentralCache要的batchNum就会越小
	// 4. size越小, 一次向CentralCache要的batchNum就会越大
	size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));

	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	// 双向链表的头尾
	void* start = nullptr;
	void* end = nullptr;
	size_t actuaNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actuaNum > 1); // 至少得有一个

	if (actuaNum == 1)
	{
		assert(start == end);
		return start;
	}
	else
	{
		_freeLists[index].PushRange(NextObj(start), end);
		return start;
	}
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