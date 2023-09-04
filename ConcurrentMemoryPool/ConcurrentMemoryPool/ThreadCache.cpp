#include "ThreadCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ...
	return nullptr;
}

void* ThreadCache::Allocate(size_t size) // ����
{
	assert(size <= MAX_BYTES); // ������������ڴ�
	size_t alignSize = SizeClass::RoundUp(size); //�����С
	size_t index = SizeClass::Index(size); // ��ϣͰ����

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

	// �Ҷ�ӳ�����������Ͱ������������
	size_t index = SizeClass::Index(size);
	_freeLists[index].Push(ptr);
}