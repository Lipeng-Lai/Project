#pragma once

#include "Common.h"

// ����ģʽ������ģʽ
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	// ��ȡһ��Kҳ��S	pan
	Span* NewSpan(size_t k);
	std::mutex _pageMtx;

private:
	SpanList _spanLists[NPAGES];

	PageCache() // ����ģʽ�����캯��˽��:���������ഴ������
	{}

	PageCache(const PageCache&) = delete; // �����캯��˽�л�

	static PageCache _sInst;
};