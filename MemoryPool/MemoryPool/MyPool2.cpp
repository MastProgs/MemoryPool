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
	// 현재 락 -> mutex
	char * pHere = m_pMain;

	// 사이즈 기록 후
	long long realSize = s + Tag_size;
	if (realSize > Max_alloc_size)
	{
		return nullptr;
	}

	// 남은 공간 확인 후
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
				// 사용중이면, 다음 공간으로 넘어간다
				long long nextDistance = (pHereVal & TAG::Mask) + Tag_size;
				pHere += nextDistance;
			}
			else
			{
				// 남은 공간이 있다 - 공간 여유가 있는지 확인 후 바로 할당 ( First-Fit 정책 )
				if ((realSize <= pHereVal) || 0 == pHereVal)
				{
					// 할당 시작
					long long lastFreeSize = pHereVal - realSize;

					// 사용 공간 외, 나머지 공간에 태그 작성이 가능한지 확인 - 9 이상은 남아야 1바이트 라도 할당 가능
					if (Tag_size + 1 < lastFreeSize)
					{
						if (false == CAS(pHere, pHereVal, (realSize - Tag_size) | TAG::isUse))
						{
							break;
						}

						// 뒤에 남은 공간 만큼 태그 작성 - CAS 실패시 어쩌지? ㅋㅋㅋㅋ 답이 없네 - 뒷수습을 해야됨 ㅠ 어디서 하나...
						// 어차피 here 가 0 이면, break 때려서 상관 없을듯?
						// LLval(pHere + realSize) = lastFreeSize;
						if (false == CAS(pHere + realSize, LLval(pHere + realSize), lastFreeSize))
						{
						}
						return pHere + Tag_size;
					}

					// 얼마 안 남은 공간 만큼 크기 그냥 사용
					if (false == CAS(pHere, pHereVal, (pHereVal - Tag_size) | TAG::isUse))
					{
						break;
					}
					///m_L.unlock();
					return pHere + Tag_size;
				}
				else
				{
					// 다음 태그로 넘어가기
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
	// 현재 락 -> mutex

	// 앞 태그 사용 해제
	char * pCurr{ (char *)p };
	char * pPreTag{ pCurr - Tag_size };
	long long memSize{ (LLval(pPreTag)) & TAG::Mask };

	///m_L.lock();
	ZeroMem(pCurr, memSize);

	// 뒷 태그 메모리 존재 여부 확인
	char * pNextTag{ pCurr + memSize };
	long long pNextTagVal = LLval(pNextTag);
	if (pNextTagVal & TAG::isUse)
	{
		// 사용중이므로 건들면 앙대는 곳
	}
	else
	{
		// 비어있는 여유 공간 - 마킹은 남은 공간 값 계산해서 넣을때, 값 계산 도중 자연스럽게 사라진다 == 이렇게 하면 안됨
		if (0 == pNextTagVal)
		{
			// 태그 자체가 없을 경우 - 이리로 오면 뭔가 이상함, 아니면 완전 메모리 끝이거나..
			//LLval(pPreTag) = (m_pEnd - pPreTag) | TAG::isUse;
			CAS(pPreTag, LLval(pPreTag), memSize | TAG::isUse);
			//return;
		}
		else
		{
			do 
			{
				pNextTag = pPreTag + ((LLval(pPreTag)) & TAG::Mask) + Tag_size;

				// 비어있는 메모리 공간이 있다면 합치자.
				long long CalculateFreeSize = LLval(pNextTag);

				// 다시 한번 더 건든 사람이 있는지 검사하기
				if (CalculateFreeSize & TAG::isUse)
				{
					// 그새 누가 사용중!!
					CalculateFreeSize = 0;
				}
				else
				{
					if (false == CAS(pNextTag, CalculateFreeSize, 0))
					{
						// 체크하고 난 뒤, 그 잠깐 찰나에 또 할당해서 써버린거라, 건들면 앙댐
						CalculateFreeSize = 0;
					}
				}

				CalculateFreeSize += memSize;
				//LLval(pPreTag) = CalculateFreeSize | TAG::isUse;
				CAS(pPreTag, LLval(pPreTag), CalculateFreeSize | TAG::isUse);

				// 사용중인 위치가 나오거나, 실제 메모리 끝이 나올때 까지 반복
			} while (!((LLval(pPreTag)) & TAG::isUse) || !(pPreTag >= m_pEnd));
		}
	}

	//LLval(pPreTag) = (LLval(pPreTag) & TAG::Mask) + Tag_size;
	CAS(pPreTag, LLval(pPreTag), (LLval(pPreTag) & TAG::Mask) + Tag_size);
	///m_L.unlock();
}
