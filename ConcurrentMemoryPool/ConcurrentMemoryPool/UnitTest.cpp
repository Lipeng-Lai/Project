#include "ObjectPool.h"
#include "ConcurrentAlloc.h"

void Alloc1()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(6);
	}
}

void Alloc2()
{
	for (size_t i = 0; i < 5; ++i)
	{
		void* ptr = ConcurrentAlloc(7);
	}
}

void TLSTest()
{
	std::thread t1(Alloc1); // 创建了线程1 去执行Alloc1函数
	t1.join();

	std::thread t2(Alloc2); // 创建了线程2 去执行Alloc2函数
	t2.join();
}

int main()
{
	//TestObjectPool();
	TLSTest();

	return 0;
}