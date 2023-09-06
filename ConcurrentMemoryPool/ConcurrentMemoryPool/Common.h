#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#include <time.h>
#include <assert.h>

#include <thread>
#include <mutex>

using std::cout;
using std::endl;

static const size_t MAX_BYTES = 256 * 1024; // 最大分配256KB的内存
static const size_t NFREELIST = 208; // 分段映射的哈希桶总数
static const size_t NPAGES = 129; // PageCache页数的最大值
static const size_t PAGE_SHIFT = 13; // 除以8KB

#ifdef _WIN32
#include <windows.h>
#else
// ...
#endif

#ifdef _WIN64
typedef unsigned long long PAGE_ID;
#elif _WIN32
typedef size_t PAGE_ID;
#else
// linux
#endif

// 直接去堆上按页申请空间
inline static void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// linux下brk mmap等
#endif

	if (ptr == nullptr)
		throw std::bad_alloc();

	return ptr;
}


static void*& NextObj(void* obj)
{
	return *(void**)obj;
}

// 管理切分好的小对象的自由链表
class FreeList
{
public:
	void Push(void* obj)
	{
		assert(obj);
		// 链表头插
		// *(void**)obj = _freeList;
		NextObj(obj) = _freeList; // 当前节点存储下一个节点的地址
		_freeList = obj; // 头节点存储当前节点的地址
	}

	void PushRange(void* start, void* end) // 将内存块(也是双向链表)连接到双向自由链表
	{
		NextObj(end) = _freeList;
		_freeList = start;
	}

	void* Pop()
	{
		assert(_freeList);
		// 链表头删
		void* obj = _freeList; // 当前节点存储下一个节点的地址
		_freeList = NextObj(obj); // 将下一个节点的地址存储到头节点中

		return obj;
	}

	bool Empty()
	{
		// 链表判空
		return _freeList == nullptr;
	}

	size_t& MaxSize()
	{
		return _maxSize;
	}

private:
	void* _freeList = nullptr;
	size_t _maxSize = 1;
};


// 计算对象大小的对齐映射规则
class SizeClass
{
public:
	// 整体控制在最多10%左右的内碎片浪费
	// [1,128]					8byte对齐	    freelist[0,16)
	// [128+1,1024]				16byte对齐	    freelist[16,72)
	// [1024+1,8*1024]			128byte对齐	    freelist[72,128)
	// [8*1024+1,64*1024]		1024byte对齐     freelist[128,184)
	// [64*1024+1,256*1024]		8*1024byte对齐   freelist[184,208)

	/* 普通写法(效率一般, 但直观)
	size_t _RoundUp(size_t size, size_t alignNum)
	{
		size_t alignSize;
		if (size % alignNum != 0)
		{
			alignSize = (size / alignNum + 1)*alignNum;
		}
		else
		{
			alignSize = size;
		}

		return alignSize;
	}*/

	// 高手的写法(位运算效率高)
	// 1-8 
	static inline size_t _RoundUp(size_t bytes, size_t alignNum)
	{
		return ((bytes + alignNum - 1) & ~(alignNum - 1));
	}

	// 按需向上分配内存
	static inline size_t RoundUp(size_t size)
	{
		if (size <= 128)
		{
			return _RoundUp(size, 8);
		}
		else if (size <= 1024)
		{
			return _RoundUp(size, 16);
		}
		else if (size <= 8 * 1024)
		{
			return _RoundUp(size, 128);
		}
		else if (size <= 64 * 1024)
		{
			return _RoundUp(size, 1024);
		}
		else if (size <= 256 * 1024)
		{
			return _RoundUp(size, 8 * 1024);
		}
		else
		{
			assert(false);
			return -1;
		}
	}


	// 寻找哈希桶对应的索引
	// 普通人的写法(效率一般，但直观)
	/*size_t _Index(size_t bytes, size_t alignNum)
	{
	if (bytes % alignNum == 0)
	{
	return bytes / alignNum - 1;
	}
	else
	{
	return bytes / alignNum;
	}
	}*/

	// 高手的位运算写法(效率高)
	static inline size_t _Index(size_t bytes, size_t align_shift)
	{
		return ((bytes + (static_cast<unsigned long long>(1) << align_shift) - 1) >> align_shift) - 1;
	}

	// 计算映射的哪一个自由链表桶
	static inline size_t Index(size_t bytes)
	{
		assert(bytes <= MAX_BYTES);

		// 每个区间有多少个链
		static int group_array[4] = { 16, 56, 56, 56 };
		if (bytes <= 128) {
			return _Index(bytes, 3);
		}
		else if (bytes <= 1024) {
			return _Index(bytes - 128, 4) + group_array[0];
		}
		else if (bytes <= 8 * 1024) {
			return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
		}
		else if (bytes <= 64 * 1024) {
			return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1] + group_array[0];
		}
		else if (bytes <= 256 * 1024) {
			return _Index(bytes - 64 * 1024, 13) + group_array[3] + group_array[2] + group_array[1] + group_array[0];
		}
		else {
			assert(false);
		}

		return -1;
	}

	// threadCache一次从CentralCache获取多少个对象
	static size_t NumMoveSize(size_t size) // size = 8bytes, 16....
	{
		assert(size > 0);
		// [2, 512] 一次批量移动多少个对象的慢启动上限值
		// 小对象一次批量上限高
		// 大对象一次批量上限低
		int num = MAX_BYTES / size;

		if (num < 2) num = 2;
		if (num > 512) num = 512;

		return num;
	}

	// 计算一次向系统获取几个页
	// 单个对象 8byte
	// ...
	// 单个对象256KB
	static size_t NumMovePage(size_t size)
	{
		size_t num = NumMoveSize(size);
		size_t npage = num * size;

		npage >>= PAGE_SHIFT;
		if (npage == 0) npage = 1; // 不足一页，按一页给

		return npage;
	}
};


// 管理多个连续页大块内存
struct Span
{
	PAGE_ID _pageId = 0; // 大块内存起始页的页号
	size_t _n = 0; // 页的数量

	Span* _next = nullptr; // 双向链表的结构
	Span* _prev = nullptr;

	size_t _useCount = 0; // 切好小内存，被分配给ThreadCache的计数
	void* _freeList = nullptr; // 切好的小块内存的自由链表
};

// 带头双向循环链表
class SpanList
{
public:
	SpanList()
	{
		_head = new Span;
		_head->_next = _head;
		_head->_prev = _head;
	}

	Span* Begin()
	{
		return _head->_next;
	}

	Span* End()
	{
		return _head;
	}

	void Insert(Span* pos, Span* newSpan)
	{
		assert(pos);
		assert(newSpan);

		Span* prev = pos->_prev; // 当前节点的前一个节点
		// 结构: prev ~ newSpan ~ pos

		// 双向链表的连接操作
		prev->_next = newSpan;
		newSpan->_prev = prev;
		newSpan->_next = pos;
		pos->_prev = newSpan;
	}

	void Erase(Span* pos)
	{
		assert(pos);
		assert(pos != _head);

		// 双向链表的删除操作
		Span* prev = pos->_prev;
		Span* next = pos->_next;

		prev->_next = next;
		next->_prev = prev;

		// 此时的删除Span，其实只是将该Span归还给下一层的PAGE_Cache，所以无需删除
	}

	void PushFront(Span* span)
	{
		Insert(Begin(), span);
	}

	Span* PopFront()
	{
		Span* front = _head->_next;
		Erase(front);
		return front;
	}

	bool Empty()
	{
		return _head->_next == _head;
	}



private:
	Span* _head;
public:
	std::mutex _mtx; // 桶锁：访问同一个桶时才需要加锁
};