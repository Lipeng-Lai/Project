#include "Common.h"
//#ifdef _WIN32
//#include<windows.h>
//#else
//// 
//#endif
//
//// 定长内存池
////template<size_t N>
////class ObjectPool
////{};
//
//// 直接去堆上按页申请空间
//inline static void* SystemAlloc(size_t kpage)
//{
//#ifdef _WIN32
//	void* ptr = VirtualAlloc(0, kpage << 13, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
//#else
//	// linux下brk mmap等
//#endif
//
//	if (ptr == nullptr)
//		throw std::bad_alloc();
//
//	return ptr;
//}

template<class T>
class ObjectPool
{
public:
	T* New()
	{
		T* obj = nullptr;

		// 优先把还回来内存块对象，再次重复利用
		if (_freeList)
		{
			// 链表头删
			void* next = *((void**)_freeList);
			obj = (T*)_freeList;
			_freeList = next;
		}
		else
		{
			// 剩余内存不够一个对象大小时，则重新开大块空间
			if (_remainBytes < sizeof(T))
			{
				_remainBytes = 128 * 1024;
				_memory = (char*)malloc(_remainBytes); // 128KB
				if (_memory == nullptr)
				{
					throw std::bad_alloc();
				}
			}
			// 判断还回来的内存大小，如果小于void*，那么至少要给一个void*大小的内存
			// 以用来存放下一个节点的地址
			obj = (T*)_memory;
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += objSize;
			_remainBytes -= objSize;
		}
		// 开完空间后，需要对这块空间进行初始化
		// 定位new，显示调用T的构造函数初始化
		new(obj)T;

		return obj;
	}

	void Delete(T* obj)
	{
		// 需要把一个节点的前4/8个位写入下一个节点的地址
		//if (_freeList == nullptr)
		//{
		//	_freeList = obj;
		//	//*(int*)obj = nullptr; 此时指针大小为4个字节 适用于32位下
		//	//*（long long*) 此时指针大小为8个字节 适用于64位下
		//	*(void**)obj = nullptr; // 二级指针可以自适应
		//}
		//else
		//{
		//	// 链表头插(因为尾插时间复杂度为O（N）
		//	*(void**)obj = _freeList;
		//	_freeList = obj;
		//}
		// 显示调用析构函数清理对象
		obj->~T();

		// 链表头插
		*(void**)obj = _freeList;
		_freeList = obj;
	}

private:
	char* _memory = nullptr; // 指向大块内存的指针
	size_t _remainBytes = 0; // 大块内存在切分过程中剩余字节数
	void* _freeList = nullptr; // 还回来过程中链接的自由链表的头指针
};

// -----------------------------------测试------------------------------------------//

struct TreeNode
{
	int _val;
	TreeNode* _left;
	TreeNode* _right;

	TreeNode()
		:_val(0)
		, _left(nullptr)
		, _right(nullptr)
	{}
};

void TestObjectPool()
{
	// 申请释放的轮次
	const size_t Rounds = 5;

	// 每轮申请释放多少次
	const size_t N = 1000000;

	std::vector<TreeNode*> v1;
	v1.reserve(N);

	size_t begin1 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v1.push_back(new TreeNode);
		}
		for (int i = 0; i < N; ++i)
		{
			delete v1[i];
		}
		v1.clear();
	}

	size_t end1 = clock();

	std::vector<TreeNode*> v2;
	v2.reserve(N);

	ObjectPool<TreeNode> TNPool;
	size_t begin2 = clock();
	for (size_t j = 0; j < Rounds; ++j)
	{
		for (int i = 0; i < N; ++i)
		{
			v2.push_back(TNPool.New());
		}
		for (int i = 0; i < N; ++i)
		{
			TNPool.Delete(v2[i]);
		}
		v2.clear();
	}
	size_t end2 = clock();

	cout << "new cost time:" << end1 - begin1 << endl;
	cout << "object pool cost time:" << end2 - begin2 << endl;
}