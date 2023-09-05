#include "ThreadCache.h"
#include "CentralCache.h"

void* ThreadCache::FetchFromCentralCache(size_t index, size_t size)
{
	// ����ʼ���������㷨
	// 1. �ʼ����һ����CentralCacheһ������Ҫ̫��, ��Ϊ������˷�
	// 2. ����㲻�������size��С�ڴ�����, ��ôbatchNum�ͻ᲻������, ֱ������512
	// 3. sizeԽ��, һ����CentralCacheҪ��batchNum�ͻ�ԽС
	// 4. sizeԽС, һ����CentralCacheҪ��batchNum�ͻ�Խ��
	size_t batchNum = std::min(_freeLists[index].MaxSize(), SizeClass::NumMoveSize(size));

	if (_freeLists[index].MaxSize() == batchNum)
	{
		_freeLists[index].MaxSize() += 1;
	}

	// ˫�������ͷβ
	void* start = nullptr;
	void* end = nullptr;
	size_t actuaNum = CentralCache::GetInstance()->FetchRangeObj(start, end, batchNum, size);
	assert(actuaNum > 1); // ���ٵ���һ��

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