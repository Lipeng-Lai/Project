#pragma once
#include "Common.h"

class ThreadCache
{
public:
	// ������ͷ��ڴ����
	void* Allocate(size_t size);
	void Deallocate(void* ptr, size_t size);

	// �����Ļ����ȡ����
	void* FetchFromCentralCache(size_t index, size_t size);
private:
	FreeList _freeLists[NFREELIST]; // ��ϣͰ: ����洢FreeList
};

// TLS thread local storage ���Ծֲ��洢
// ��һ�ֱ����Ĵ洢��ʽ����������������ڵ��߳����ǿ�ȫ�ַ��ʵģ����ǲ��ܱ������̷߳��ʵ�
static _declspec(thread) ThreadCache* pTLSthreadCache = nullptr;