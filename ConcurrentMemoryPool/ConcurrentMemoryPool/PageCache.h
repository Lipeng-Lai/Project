#pragma once

#include "Common.h"

// 单例模式：饿汉模式
class PageCache
{
public:
	static PageCache* GetInstance()
	{
		return &_sInst;
	}

	// 获取一个K页的S	pan
	Span* NewSpan(size_t k);
	std::mutex _pageMtx;

private:
	SpanList _spanLists[NPAGES];

	PageCache() // 单例模式将构造函数私有:不想让其余创建对象
	{}

	PageCache(const PageCache&) = delete; // 将构造函数私有化

	static PageCache _sInst;
};