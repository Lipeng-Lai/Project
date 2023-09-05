#pragma once

#include "Common.h"

// ����ģʽ������ģʽ
class CentralCache
{
public:
	static CentralCache* GetInstance() // ��ȡʵ������
	{
		return &_sInst;
	}

	// ��ȡһ���ǿյ�Span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	// ��CentralCache��ȡһ�������Ķ����ThreadCache
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

private:
	SpanList _spanLists[NFREELIST];

private:
	CentralCache() // ����ģʽ�����캯��˽��:���������ഴ������
	{}

	CentralCache(const CentralCache&) = delete; // �����캯��˽�л�

	static CentralCache _sInst;
};