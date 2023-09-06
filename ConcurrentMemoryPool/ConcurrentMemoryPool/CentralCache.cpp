#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

// 获取一个非空的span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// 查看当前的SpanList中是否还有未分配的span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr) return it;
		else it = it->_next;
	}

	// 先把CentralCache的桶锁解掉: 因为如果其他线程释放内存独享回来，不会被阻塞
	list._mtx.unlock();

	// 走到这里说明没有空闲的Span 只能找PageCache
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// 对获取span进行切分，不需要加锁，因为此时其他线程访问不到这个span


	// 计算Span的大块内存的起始地址和大块内存的大小(字节数)
	char* start = (char*)(span->_pageId << PAGE_SHIFT); // 页数 * 8KB
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// 把大块内存切成自由链表链接起来
	// 1. 先切一块下来做头 方便后续尾插(保证地址的连续性)

	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;

	while (start < end)
	{
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}

	// 切好span以后，需要把span挂到桶里面时，需要加锁
	list._mtx.lock();

	list.PushFront(span);

	return span;
}

// 从中心缓存获取一定数量的对象给thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// 从span中获取batchNum个对象
	// 如果不够batchNum个，有多少拿多少
	start = span->_freeList;
	end = start;
	size_t i = 0;
	size_t actualNum = 1;
	while (i < batchNum - 1 && NextObj(end) != nullptr)
	{
		end = NextObj(end);
		++i;
		++actualNum;
	}
	span->_freeList = NextObj(end);
	NextObj(end) = nullptr;

	_spanLists[index]._mtx.unlock();

	return actualNum;
}