#include "MyPool2.h"
#include <iostream>


MyPool2::MyPool2()
{
	m_pMain = (char *)malloc(Max_mem_size);
	//m_pIter = m_pMain;
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
	//short retry{ 2 };
	///m_L.lock();
	do
	{
		long long preTagToNextDistance = LLval(pHere) & TAG::Mask;
		while (true)
		{
			if (pHere >= m_pEnd)
			{
				pHere = m_pMain;
				break;
			}

			if (LLval(pHere) == 0)
			{
				break;
			}

			long long pHereVal = LLval(pHere);
			if (pHereVal & TAG::isUse)
			{
				// ������̸�, ���� �������� �Ѿ��
				long long nextDistance = (pHereVal & TAG::Mask) + Tag_size;
				pHere += nextDistance;
			}
			else
			{
				// ���� ������ �ִ� - ���� ������ �ִ��� Ȯ�� �� �ٷ� �Ҵ� ( First-Fit ��å )
				if ((realSize <= pHereVal) || 0 == pHereVal)
				{
					// �Ҵ� ����
					long long lastFreeSize = pHereVal - realSize;

					// ��� ���� ��, ������ ������ �±� �ۼ��� �������� Ȯ�� - 9 �̻��� ���ƾ� 1����Ʈ �� �Ҵ� ����
					if (Tag_size + 1 < lastFreeSize)
					{
						if (false == CAS(pHere, pHereVal, (realSize - Tag_size) | TAG::isUse))
						{
							break;
						}

						// �ڿ� ���� ���� ��ŭ �±� �ۼ� - CAS ���н� ��¼��? �������� ���� ���� - �޼����� �ؾߵ� �� ��� �ϳ�...
						// ������ here �� 0 �̸�, break ������ ��� ������?
						// LLval(pHere + realSize) = lastFreeSize;
						if (false == CAS(pHere + realSize, LLval(pHere + realSize), lastFreeSize))
						{
						}
						return pHere + Tag_size;
					}

					// �� �� ���� ���� ��ŭ ũ�� �׳� ���
					if (false == CAS(pHere, pHereVal, (pHereVal - Tag_size) | TAG::isUse))
					{
						break;
					}
					///m_L.unlock();
					return pHere + Tag_size;
				}
				else
				{
					// ���� �±׷� �Ѿ��
					pHere += pHereVal;
				}
			}
		}
	} while (true);

	///m_L.unlock();
	return nullptr;
}

void MyPool2::Free(void * p)
{
	// ���� �� -> mutex

	// �� �±� ��� ����
	char * pCurr{ (char *)p };
	char * pPreTag{ pCurr - Tag_size };
	long long memSize{ (LLval(pPreTag)) & TAG::Mask };

	///m_L.lock();
	ZeroMem(pCurr, memSize);

	// �� �±� �޸� ���� ���� Ȯ��
	char * pNextTag{ pCurr + memSize };
	long long pNextTagVal = LLval(pNextTag);
	if (pNextTagVal & TAG::isUse)
	{
		// ������̹Ƿ� �ǵ�� �Ӵ�� ��
	}
	else
	{
		// ����ִ� ���� ���� - ��ŷ�� ���� ���� �� ����ؼ� ������, �� ��� ���� �ڿ������� ������� == �̷��� �ϸ� �ȵ�
		if (0 == pNextTagVal)
		{
			// �±� ��ü�� ���� ��� - �̸��� ���� ���� �̻���, �ƴϸ� ���� �޸� ���̰ų�..
			//LLval(pPreTag) = (m_pEnd - pPreTag) | TAG::isUse;
			CAS(pPreTag, LLval(pPreTag), memSize | TAG::isUse);
			//return;
		}
		else
		{
			do 
			{
				pNextTag = pPreTag + ((LLval(pPreTag)) & TAG::Mask) + Tag_size;

				// ����ִ� �޸� ������ �ִٸ� ��ġ��.
				long long CalculateFreeSize = LLval(pNextTag);

				// �ٽ� �ѹ� �� �ǵ� ����� �ִ��� �˻��ϱ�
				if (CalculateFreeSize & TAG::isUse)
				{
					// �׻� ���� �����!!
					CalculateFreeSize = 0;
				}
				else
				{
					if (false == CAS(pNextTag, CalculateFreeSize, 0))
					{
						// üũ�ϰ� �� ��, �� ��� ������ �� �Ҵ��ؼ� ������Ŷ�, �ǵ�� �Ӵ�
						CalculateFreeSize = 0;
					}
				}

				CalculateFreeSize += memSize;
				//LLval(pPreTag) = CalculateFreeSize | TAG::isUse;
				CAS(pPreTag, LLval(pPreTag), CalculateFreeSize | TAG::isUse);

				// ������� ��ġ�� �����ų�, ���� �޸� ���� ���ö� ���� �ݺ�
			} while (!((LLval(pPreTag)) & TAG::isUse) || !(pPreTag >= m_pEnd));
		}
	}

	//LLval(pPreTag) = (LLval(pPreTag) & TAG::Mask) + Tag_size;
	CAS(pPreTag, LLval(pPreTag), (LLval(pPreTag) & TAG::Mask) + Tag_size);
	///m_L.unlock();
}
