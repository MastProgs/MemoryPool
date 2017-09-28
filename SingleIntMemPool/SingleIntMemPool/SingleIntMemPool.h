#pragma once
#include <iostream>

using Node = struct node
{
	size_t mallocSize{ 0 };
	int * pMain{ nullptr };
	node * pLeft{ nullptr };
	node * pRight{ nullptr };

	node(size_t s)
		: mallocSize{ s }
	{
		if (s > 0)
		{
			pMain = (int *)malloc(sizeof(int) * s);
		}
	}
	~node() { free(pMain); }
};

class MyBinary
{
public:
	MyBinary() { Init(); }
	~MyBinary() { Release(); }

	void Add(Node * const n)
	{
		n->pRight = nullptr;
		n->pLeft = nullptr;

		++m_size;
		if (nullptr == m_pRoot)
		{
			m_pRoot = n;
		}

		Node * p = m_pRoot;
		Node ** pPrevLR;
		while (nullptr != p)
		{
			if (p->mallocSize < n->mallocSize)
			{
				pPrevLR = &p->pRight;
				p = p->pRight;
			}
			else
			{
				pPrevLR = &p->pLeft;
				p = p->pLeft;
			}
		}
		*pPrevLR = n;
	};

	Node * Pop(Node * pPopNode, Node ** pParrentLRtoModify)
	{
		if (nullptr == pPopNode)
		{
			return nullptr;
		}
		/// 작은 애가 pPrev 값으로 들어가고
		/// 큰 녀석이 우측에 달리기
		--m_size;

		*pParrentLRtoModify = pPopNode->pLeft;
		if (nullptr == *pParrentLRtoModify)
		{
			*pParrentLRtoModify = pPopNode->pRight;
		}
		pPopNode->pRight = nullptr;
		pPopNode->pLeft = nullptr;

		return pPopNode;
	};

	Node * FindSizeFit(const size_t & s, Node ** pParrentLR)
	{
		// malloc Size 로 맞는 공간을 찾아주자, 딱 맞을 필요 없고 큰공간주기.
		if (0 == m_size)
		{
			// 없으면 새로 할당
			pParrentLR = nullptr;
			return nullptr;
		}

		Node * pCurr = m_pRoot;
		Node * pNextNode = pCurr->pRight;
		while (nullptr != pNextNode)
		{
			if (pNextNode->mallocSize < s)
			{
				pParrentLR = &pCurr->pRight;
				pCurr = pNextNode;
				pNextNode = pNextNode->pRight;
			}
			else
			{
				// 그냥 큰 사이즈니까 반환해버리자
				if (pCurr->pLeft != nullptr)
				{
					// 근데 위에 노드로 돌아갈 방법이 없어...
				}
				break;
			}
		}

		if (pCurr->mallocSize < s)
		{
			return nullptr;
		}
		else
		{
			pParrentLR = nullptr;
			return pCurr;
		}
	};

	// 사용중인 노드 분류에서만 쓰는 함수
	Node * FindIntPtr(const int * const p, Node ** pParrentLR)
	{
		if (0 == m_size)
		{
			// 찾으려는 int 포인터가 없으면 뭔가 이상한건뎅.. 쓰는지 안쓰는지만 보는거니까..
			pParrentLR = nullptr;
			return nullptr;
		}
		return IntPtrFind(m_pRoot, p, &pParrentLR);
	}

	// 사용중인 노드 분류에서만 쓰는 함수
	Node * IntPtrFind(Node * pNode, const int * const p, Node *** pParrentLR)
	{
		if (pNode == nullptr)
		{
			return nullptr;
		}

		if (pNode->pMain == p)
		{
			return pNode;
		}

		Node * pReturn{ nullptr };
		pReturn = IntPtrFind(pNode->pLeft, p, pParrentLR);
		if (nullptr != pReturn)
		{
			*pParrentLR = &pNode->pLeft;
			return pReturn;
		}
		pReturn = IntPtrFind(pNode->pRight, p, pParrentLR);
		*pParrentLR = &pNode->pRight;
		return pReturn;
	}

	size_t getSize() const { return m_size; }

private:
	Node * m_pRoot{ nullptr };
	size_t m_size{ 0 };

	void Init()
	{
		m_pRoot = new Node{ 0 };
		m_pRoot->mallocSize = 0;
	}

	void Release()
	{
		while (m_size)
		{
			Node ** pParrentLR{ nullptr };
			Node * p = FindSizeFit(1, pParrentLR);
			free(Pop(p, pParrentLR));
		}
		free(m_pRoot);
	}
};

class MyList
{
public:
	MyList() { Init(); }
	~MyList() { Release(); }

	void Add(Node * const n)
	{
		if (m_size == 0)
		{
			m_pTailNext = &m_pHead->pRight;
		}
		n->pRight = nullptr;
		n->pLeft = nullptr;

		++m_size;
		*m_pTailNext = n;
		m_pTailNext = &n->pRight;
	};

	Node * Pop()
	{
		Node * p = m_pHead->pRight;
		if (p != nullptr)
		{
			--m_size;
			m_pHead->pRight = m_pHead->pRight->pRight;
		}
		else
		{
			m_pTailNext = &m_pHead->pRight;
		}

		return p;
	};

	Node * FindIntPtrAndPop(int * p)
	{
		Node * ptr = m_pHead;
		Node * pPrev = ptr;
		while (ptr != nullptr)
		{
			if (ptr->pMain == p)
			{
				pPrev->pRight = ptr->pRight;
				ptr->pRight = nullptr;
				--m_size;
				return ptr;
			}
			else
			{
				pPrev = ptr;
				ptr = ptr->pRight;
			}
		}
		return nullptr;
	}

	Node * FindSizeFit(size_t s)
	{
		Node * pPrev = m_pHead;
		Node * pCurr = pPrev->pRight;

		if (pCurr == nullptr)
		{
			return nullptr;
		}

		while (pCurr->pRight != nullptr)
		{
			if (pCurr->mallocSize >= s)
			{
				--m_size;
				pPrev->pRight = pCurr->pRight;
				return pCurr;
			}
			pPrev = pCurr;
			pCurr = pCurr->pRight;
		}

		return nullptr;
	}

	size_t getSize() const { return m_size; }

private:
	Node * m_pHead{ nullptr };
	Node ** m_pTailNext{ nullptr };

	size_t m_size{ 0 };

	void Init()
	{
		m_pHead = new Node{ 0 };
		m_pHead->mallocSize = 0;
		m_pHead->pRight = nullptr;
		m_pTailNext = &m_pHead->pRight;
	}

	void Release()
	{
		// 모두 해제
		while (m_pHead->pRight != nullptr)
		{
			Node * pNext = m_pHead->pRight;
			free(m_pHead);
			m_pHead = pNext;
		}
		free(m_pHead);
	}
};

class SingleIntMemPool
{
public:
	SingleIntMemPool();
	~SingleIntMemPool();

	int * Malloc(size_t s);
	void Free(int * p);

private:
	int * pHead{ nullptr };
	int * pTail{ nullptr };

	MyList m_freeInt;
	MyList m_freeArr;
	MyList m_using;

	size_t m_allNodeCnt{ 0 };

	void Init();
	void Release();
};

