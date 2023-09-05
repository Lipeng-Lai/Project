#pragma once
#include "Common.h"

class ThreadCache
{
public:
	// 申请和释放内存对象
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	// 从中心缓存获取对象
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	FreeList _freeLists[NFREELIST]; // 哈希桶: 分类存储FreeList
};

// TLS thread local storage 线性局部存储
// 是一种变量的存储方式，这个变量在它所在的线程内是可全局访问的，但是不能被其他线程访问到
static _declspec(thread) ThreadCache* pTLSthreadCache = nullptr;