#include "PageCache.h"

PageCache PageCache::_sInst;

// 获取一个K页的span
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0 && k < NPAGES);

	// 检查第K个桶里面有没有span
	if (!_spanLists[k].Empty())
	{
		return _spanLists->PopFront(); // 如果不是空，那么则弹出一个Span
	}

	// 检查后面的桶里面有没有Span，如果有则可以把它切分
	// 后面如果有: 则切分成一个K页的span和一个N-K页的span
	// 其中将K页的Span送到CentralCache，N-K页的Span挂到N-K个桶(页)
	for (size_t i = k + 1; i < NPAGES; i++)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nSpan = _spanLists[i].PopFront();
			Span* kSpan = new Span;

			// 在nSpan的头部切一个K页下来

			// 举一个例子: 页号为 100，此时有页数50
			kSpan->_pageId = nSpan->_pageId; // 从页号为100开始切
			kSpan->_n = k; // 切K个页数

			nSpan->_pageId += k; // 此时页号从 100 + K 开始计数
			nSpan->_n -= k; // 但是此时 页号 100 + K 还剩余 50 - K个页数

			// N-K页的Span挂到N-K个桶(页号)
			_spanLists[nSpan->_n].PushFront(nSpan);

			return kSpan; // 其中将K页的Span送到CentralCache
		}
	}

	// 走到这个位置就说明后面没有大页的span
	// 此时向堆要一个128页的span

	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1); // 数组映射 -1
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT; // 页号 = 除以8K 
	bigSpan->_n = NPAGES - 1; // 页数

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}