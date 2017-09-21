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
	// 현재 락 -> mutex
	char * pHere = m_pMain;

	// 사이즈 기록 후
	long long realSize = s + Tag_size;
	if (realSize > Max_alloc_size)
	{
		return nullptr;
	}

	// 남은 공간 확인 후
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
				// 사용중이면, 다음 공간으로 넘어간다
				long long nextDistance = (LLval(pHere) & TAG::Mask) + Tag_size;
				pHere += nextDistance;
			}
			else
			{
				// 남은 공간이 있다 - 공간 여유가 있는지 확인 후 바로 할당 ( First-Fit 정책 )
				if ((realSize < LLval(pHere)) || 0 == LLval(pHere))
				{
					// 할당
					long long lastFreeSize = LLval(pHere) - realSize;

					// 사용 공간 외, 나머지 공간에 태그 작성이 가능한지 확인 - 9 이상은 남아야 1바이트 라도 할당 가능
					if (Tag_size + 1 < lastFreeSize)
					{
						LLval(pHere) = (realSize - Tag_size) | TAG::isUse;
						void * retPtr = pHere + Tag_size;

						// 뒤에 남은 공간 만큼 태그 작성
						pHere += realSize;
						LLval(pHere) = lastFreeSize;

						m_L.unlock();
						return retPtr;
					}

					// 얼마 안 남은 공간 만큼 크기 그냥 사용
					LLval(pHere) = (LLval(pHere) - Tag_size) | TAG::isUse;
					void * retPtr = pHere + Tag_size;

					m_L.unlock();
					return retPtr;
				}
				else
				{
					// 다음 태그로 넘어가기
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
	// 현재 락 -> mutex

	// 앞 태그 사용 해제
	char * pCurr{ (char *)p };
	char * pPreTag{ pCurr - Tag_size };
	long long memSize{ (LLval(pPreTag)) & TAG::Mask };

	m_L.lock();
	ZeroMem(pCurr, memSize);

	// 뒷 태그 메모리 존재 여부 확인
	char * pNextTag{ pCurr + memSize };
	if ((LLval(pNextTag)) & TAG::isUse)
	{
		// 사용중이므로 건들면 앙대는 곳
	}
	else
	{
		// 비어있는 여유 공간
		if (0 == (*(long long *)pNextTag))
		{
			// 태그 자체가 없을 경우
			LLval(pPreTag) = m_pEnd - pPreTag;
		}
		else
		{
			// 비어있는 메모리 공간이 있다면 합치자.
			LLval(pPreTag) = (LLval(pNextTag) + memSize);
			ZeroMem(pNextTag, Tag_size);
		}
	}
	
	//std::cout << LLval(pPreTag) << std::endl;
	LLval(pPreTag) = (LLval(pPreTag) & TAG::Mask) + Tag_size;

	m_L.unlock();
}
