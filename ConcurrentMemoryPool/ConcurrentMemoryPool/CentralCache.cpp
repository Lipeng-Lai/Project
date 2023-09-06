#include "CentralCache.h"
#include "PageCache.h"

CentralCache CentralCache::_sInst;

// ��ȡһ���ǿյ�span
Span* CentralCache::GetOneSpan(SpanList& list, size_t size)
{
	// �鿴��ǰ��SpanList���Ƿ���δ�����span
	Span* it = list.Begin();
	while (it != list.End())
	{
		if (it->_freeList != nullptr) return it;
		else it = it->_next;
	}

	// �Ȱ�CentralCache��Ͱ�����: ��Ϊ��������߳��ͷ��ڴ������������ᱻ����
	list._mtx.unlock();

	// �ߵ�����˵��û�п��е�Span ֻ����PageCache
	PageCache::GetInstance()->_pageMtx.lock();
	Span* span = PageCache::GetInstance()->NewSpan(SizeClass::NumMovePage(size));
	PageCache::GetInstance()->_pageMtx.unlock();

	// �Ի�ȡspan�����з֣�����Ҫ��������Ϊ��ʱ�����̷߳��ʲ������span


	// ����Span�Ĵ���ڴ����ʼ��ַ�ʹ���ڴ�Ĵ�С(�ֽ���)
	char* start = (char*)(span->_pageId << PAGE_SHIFT); // ҳ�� * 8KB
	size_t bytes = span->_n << PAGE_SHIFT;
	char* end = start + bytes;

	// �Ѵ���ڴ��г�����������������
	// 1. ����һ��������ͷ �������β��(��֤��ַ��������)

	span->_freeList = start;
	start += size;
	void* tail = span->_freeList;

	while (start < end)
	{
		NextObj(tail) = start;
		tail = NextObj(tail);
		start += size;
	}

	// �к�span�Ժ���Ҫ��span�ҵ�Ͱ����ʱ����Ҫ����
	list._mtx.lock();

	list.PushFront(span);

	return span;
}

// �����Ļ����ȡһ�������Ķ����thread cache
size_t CentralCache::FetchRangeObj(void*& start, void*& end, size_t batchNum, size_t size)
{
	size_t index = SizeClass::Index(size);
	_spanLists[index]._mtx.lock();

	Span* span = GetOneSpan(_spanLists[index], size);
	assert(span);
	assert(span->_freeList);

	// ��span�л�ȡbatchNum������
	// �������batchNum�����ж����ö���
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