#pragma once

#include "Common.h"

// 单例模式：饿汉模式
class CentralCache
{
public:
	static CentralCache* GetInstance() // 获取实例对象
	{
		return &_sInst;
	}

	// 获取一个非空的Span
	Span* GetOneSpan(SpanList& list, size_t byte_size);

	// 从CentralCache获取一定数量的对象给ThreadCache
	size_t FetchRangeObj(void*& start, void*& end, size_t n, size_t byte_size);

private:
	SpanList _spanLists[NFREELIST];

private:
	CentralCache() // 单例模式将构造函数私有:不想让其余创建对象
	{}

	CentralCache(const CentralCache&) = delete; // 将构造函数私有化

	static CentralCache _sInst;
};