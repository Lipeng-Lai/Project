#pragma once

#include "Common.h"
#include "ThreadCache.h"

static void* ConcurrentAlloc(size_t size)
{
	// ͨ��TLS ÿ���߳������ػ�ȡ�Լ�ר����ThreadCache����
	if (pTLSthreadCache == nullptr)
	{
		pTLSthreadCache = new ThreadCache;
	}

	cout << std::this_thread::get_id() << ":" << pTLSthreadCache << endl;

	return pTLSthreadCache->Allocate(size);
}

static void ConcurrentFree(void* ptr, size_t size)
{
	// ÿ�������ͷŶ�Ӧ��TLS ��ô��һ��ӵ��TLS�Ҳ�Ϊ��
	assert(pTLSthreadCache);
	
	pTLSthreadCache->Deallocate(ptr, size);
}