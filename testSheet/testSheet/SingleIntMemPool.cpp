#include "SingleIntMemPool.h"



SingleIntMemPool::SingleIntMemPool()
{
	Init();
}


SingleIntMemPool::~SingleIntMemPool()
{
	Release();
}

// s 는 int 갯 수로 판정한다. ( Byte 아님 )
int * SingleIntMemPool::Malloc(size_t s)
{
	Node * pNode{ nullptr };
	if (s > 1)
	{
		//Node ** pptr{ nullptr };
		pNode = m_freeArr.FindSizeFit(s);
		if (nullptr == pNode)
		{
			pNode = new Node{ s };
		}
	}
	else
	{
		pNode = m_freeInt.Pop();
		if (nullptr == pNode)
		{
			int nextSize = m_allNodeCnt;
			for (int i = 0; i < nextSize; ++i)
			{
				m_freeInt.Add(new Node{ 1 });
				++m_allNodeCnt;
			}

			pNode = m_freeInt.Pop();
		}

	}

	m_using.Add(pNode);
	return pNode->pMain;
}

void SingleIntMemPool::Free(int * p)
{
	Node * pNode{ nullptr };
	Node * pptr{ nullptr };
	pNode = m_using.FindIntPtrAndPop(p);

	if (pNode->mallocSize > 1)
	{
		m_freeArr.Add(pNode);
	}
	else
	{
		m_freeInt.Add(pNode);
	}

	// 메모리 여유 공간이 얼마나 많은지 체크해서 줄이기
	if (m_using.getSize() < (m_allNodeCnt / 4))
	{
		// 절반씩 지우자
		size_t cnt = m_freeInt.getSize() / 2;
		for (size_t i = 0; i < cnt; ++i)
		{
			free(m_freeInt.Pop());
		}

		cnt = m_freeArr.getSize() / 2;
		for (size_t i = 0; i < cnt; ++i)
		{
			free(m_freeArr.Pop());
		}
	}
}

void SingleIntMemPool::Init()
{
	m_freeInt.Add(new Node{ 1 });
	m_freeInt.Add(new Node{ 1 });
	m_allNodeCnt += 2;
}

void SingleIntMemPool::Release()
{
	// 모든 메모리 해제
}
