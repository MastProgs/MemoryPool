#include "MyPool2.h"
#include <iostream>


MyPool2::MyPool2()
{
	m_pMain = (char *)malloc(Max_mem_size);
	m_pIter = m_pMain;
	m_pEnd = (char *)m_pMain + Max_mem_size;
	//memset(pMain, 0, Max_mem_size);
	ZeroMem(m_pMain, Max_mem_size);

	LLval(m_pMain) = Max_mem_size - Tag_size;
}


MyPool2::~MyPool2()
{
	free(m_pMain);
}


void* MyPool2::Malloc(const size_t & s)
{
	// ���� �� -> mutex

	// ������ ��� ��
	long long realSize = s + Tag_size;
	if (realSize > Max_alloc_size)
	{
		return nullptr;
	}

	// ���� ���� Ȯ�� ��
	short retry{ 2 };
	m_L.lock();
	do
	{
		while (true)
		{
			if ((m_pIter == m_pEnd) || (LLval(m_pIter) == 0))
			{
				m_pIter = m_pMain;
				break;
			}

			if (LLval(m_pIter) & TAG::isUse)
			{
				// ������̸�, ���� �������� �Ѿ��
				long long nextDistance = (LLval(m_pIter) & TAG::Mask) + Tag_size;
				m_pIter += nextDistance;				
			}
			else
			{
				// ���� ������ �ִ� - ���� ������ �ִ��� Ȯ�� �� �ٷ� �Ҵ� ( First-Fit ��å )
				if ((realSize < LLval(m_pIter)) || 0 == LLval(m_pIter))
				{
					// �Ҵ�
					long long lastFreeSize = LLval(m_pIter) - realSize;

					// ��� ���� ��, ������ ������ �±� �ۼ��� �������� Ȯ�� - 9 �̻��� ���ƾ� 1����Ʈ �� �Ҵ� ����
					if (Tag_size + 1 < lastFreeSize)
					{
						LLval(m_pIter) = (realSize - Tag_size) | TAG::isUse;
						void * retPtr = m_pIter + Tag_size;

						// �ڿ� ���� ���� ��ŭ �±� �ۼ�
						m_pIter += realSize;
						LLval(m_pIter) = lastFreeSize;

						m_L.unlock();
						return retPtr;
					}

					// �� �� ���� ���� ��ŭ ũ�� �׳� ���
					LLval(m_pIter) = (LLval(m_pIter) - Tag_size) | TAG::isUse;
					void * retPtr = m_pIter + Tag_size;

					m_L.unlock();
					return retPtr;
				}
				else
				{
					// ���� �±׷� �Ѿ��
					m_pIter += LLval(m_pIter);
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
			LLval(pPreTag) = (LLval(pNextTag) + Tag_size + memSize);
			ZeroMem(pNextTag, Tag_size);
		}
	}

	LLval(pPreTag) &= TAG::Mask;
	m_L.unlock();
}
