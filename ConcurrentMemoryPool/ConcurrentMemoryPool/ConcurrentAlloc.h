#pragma once

#include "Common.h"
#include "ThreadCache.h"

static void* ConcurrentAlloc(size_t size)
{
	// 通过TLS 每个线程无锁地获取自己专属地ThreadCache对象
	if (pTLSthreadCache == nullptr)
	{
		pTLSthreadCache = new ThreadCache;
	}

	cout << std::this_thread::get_id() << ":" << pTLSthreadCache << endl;

	return pTLSthreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr, size_t size)
{
	// 每个进程释放对应的TLS 那么其一定拥有TLS且不为空
	assert(pTLSthreadCache);
	
	pTLSthreadCache->Deallocate(ptr, size);
}