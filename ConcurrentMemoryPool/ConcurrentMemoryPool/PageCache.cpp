#include "PageCache.h"

PageCache PageCache::_sInst;

// ��ȡһ��Kҳ��span
Span* PageCache::NewSpan(size_t k)
{
	assert(k > 0 && k < NPAGES);

	// ����K��Ͱ������û��span
	if (!_spanLists[k].Empty())
	{
		return _spanLists->PopFront(); // ������ǿգ���ô�򵯳�һ��Span
	}

	// �������Ͱ������û��Span�����������԰����з�
	// ���������: ���зֳ�һ��Kҳ��span��һ��N-Kҳ��span
	// ���н�Kҳ��Span�͵�CentralCache��N-Kҳ��Span�ҵ�N-K��Ͱ(ҳ)
	for (size_t i = k + 1; i < NPAGES; i++)
	{
		if (!_spanLists[i].Empty())
		{
			Span* nSpan = _spanLists[i].PopFront();
			Span* kSpan = new Span;

			// ��nSpan��ͷ����һ��Kҳ����

			// ��һ������: ҳ��Ϊ 100����ʱ��ҳ��50
			kSpan->_pageId = nSpan->_pageId; // ��ҳ��Ϊ100��ʼ��
			kSpan->_n = k; // ��K��ҳ��

			nSpan->_pageId += k; // ��ʱҳ�Ŵ� 100 + K ��ʼ����
			nSpan->_n -= k; // ���Ǵ�ʱ ҳ�� 100 + K ��ʣ�� 50 - K��ҳ��

			// N-Kҳ��Span�ҵ�N-K��Ͱ(ҳ��)
			_spanLists[nSpan->_n].PushFront(nSpan);

			return kSpan; // ���н�Kҳ��Span�͵�CentralCache
		}
	}

	// �ߵ����λ�þ�˵������û�д�ҳ��span
	// ��ʱ���Ҫһ��128ҳ��span

	Span* bigSpan = new Span;
	void* ptr = SystemAlloc(NPAGES - 1); // ����ӳ�� -1
	bigSpan->_pageId = (PAGE_ID)ptr >> PAGE_SHIFT; // ҳ�� = ����8K 
	bigSpan->_n = NPAGES - 1; // ҳ��

	_spanLists[bigSpan->_n].PushFront(bigSpan);

	return NewSpan(k);
}