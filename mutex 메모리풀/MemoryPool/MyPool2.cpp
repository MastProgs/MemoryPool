#include "MyPool2.h"
#include <iostream>


MyPool2::MyPool2()
{
	m_pMain = (char *)malloc(Max_mem_size);
	m_pIter = m_pMain;
	m_pEnd = (char *)m_pMain + Max_mem_size;
	//memset(pMain, 0, Max_mem_size);
	ZeroMem(m_pMain, Max_mem_size);

	LLval(m_pMain) = Max_mem_size;
}


MyPool2::~MyPool2()
{
	free(m_pMain);
}


void* MyPool2::Malloc(const size_t & s)
{
	// ���� �� -> mutex
	char * pHere = m_pMain;

	// ������ ��� ��
	long long realSize = s + Tag_size;
	if (realSize > Max_alloc_size)
	{
		return nullptr;
	}

	// ���� ���� Ȯ�� ��
	short retry{ 1000 };
	m_L.lock();
	do
	{
		while (true)
		{
			if ((pHere == m_pEnd) || (LLval(pHere) == 0))
			{
				pHere = m_pMain;
				break;
			}

			if (LLval(pHere) & TAG::isUse)
			{
				// ������̸�, ���� �������� �Ѿ��
				long long nextDistance = (LLval(pHere) & TAG::Mask) + Tag_size;
				pHere += nextDistance;
			}
			else
			{
				// ���� ������ �ִ� - ���� ������ �ִ��� Ȯ�� �� �ٷ� �Ҵ� ( First-Fit ��å )
				if ((realSize < LLval(pHere)) || 0 == LLval(pHere))
				{
					// �Ҵ�
					long long lastFreeSize = LLval(pHere) - realSize;

					// ��� ���� ��, ������ ������ �±� �ۼ��� �������� Ȯ�� - 9 �̻��� ���ƾ� 1����Ʈ �� �Ҵ� ����
					if (Tag_size + 1 < lastFreeSize)
					{
						LLval(pHere) = (realSize - Tag_size) | TAG::isUse;
						void * retPtr = pHere + Tag_size;

						// �ڿ� ���� ���� ��ŭ �±� �ۼ�
						pHere += realSize;
						LLval(pHere) = lastFreeSize;

						m_L.unlock();
						return retPtr;
					}

					// �� �� ���� ���� ��ŭ ũ�� �׳� ���
					LLval(pHere) = (LLval(pHere) - Tag_size) | TAG::isUse;
					void * retPtr = pHere + Tag_size;

					m_L.unlock();
					return retPtr;
				}
				else
				{
					// ���� �±׷� �Ѿ��
					pHere += (LLval(pHere) & TAG::Mask);
				}
			}
		}
	} while (--retry);

	m_L.unlock();
	return nullptr;
}

void MyPool2::Free(void * p)
{
	// ���� �� -> mutex

	// �� �±� ��� ����
	char * pCurr{ (char *)p };
	char * pPreTag{ pCurr - Tag_size };
	long long memSize{ (LLval(pPreTag)) & TAG::Mask };

	m_L.lock();
	ZeroMem(pCurr, memSize);

	// �� �±� �޸� ���� ���� Ȯ��
	char * pNextTag{ pCurr + memSize };
	if ((LLval(pNextTag)) & TAG::isUse)
	{
		// ������̹Ƿ� �ǵ�� �Ӵ�� ��
	}
	else
	{
		// ����ִ� ���� ����
		if (0 == (*(long long *)pNextTag))
		{
			// �±� ��ü�� ���� ���
			LLval(pPreTag) = m_pEnd - pPreTag;
		}
		else
		{
			// ����ִ� �޸� ������ �ִٸ� ��ġ��.
			LLval(pPreTag) = (LLval(pNextTag) + memSize);
			ZeroMem(pNextTag, Tag_size);
		}
	}
	
	//std::cout << LLval(pPreTag) << std::endl;
	LLval(pPreTag) = (LLval(pPreTag) & TAG::Mask) + Tag_size;

	m_L.unlock();
}
