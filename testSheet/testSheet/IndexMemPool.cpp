#include "IndexMemPool.h"
#include <stdlib.h>

IndexMemPool::IndexMemPool()
{
	Init();
}


IndexMemPool::~IndexMemPool()
{
	Release();
}

void IndexMemPool::Init()
{
	size_t val = MIN_MEM_SIZE;
	size_t ss{ 2560000 };
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		/*m_memSize[i] = val;
		m_memPtrArr[i] = malloc(val * MAX_MEM_CHUNCK_CNT);
		val <<= 1;*/
		m_memSize[i] = val;
		m_memPtrArr[i] = malloc(ss);
		m_totalMemSize += ss;
		ss *= 2;
	}
}

void IndexMemPool::Release()
{
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		free(m_memPtrArr[i]);
	}
}

const size_t IndexMemPool::getChunckSize(const size_t & s)
{
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		if (m_memSize[i] - s < 0)
		{
			continue;
		}
		return m_memSize[i];
	}
	return -1;
}

const int IndexMemPool::getIndexSize(const size_t & s)
{
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		if (m_memSize[i] - s < 0)
		{
			continue;
		}
		return i;
	}

	return -1;
}

const long long IndexMemPool::getPtrDistance(const void * const p, const int & index)
{
	switch (index)
	{
	case 0: return (BYTE00004 *)p - (BYTE00004 *)m_memPtrArr[index];
	case 1: return (BYTE00008 *)p - (BYTE00008 *)m_memPtrArr[index];
	case 2: return (BYTE00016 *)p - (BYTE00016 *)m_memPtrArr[index];
	case 3: return (BYTE00032 *)p - (BYTE00032 *)m_memPtrArr[index];
	case 4: return (BYTE00064 *)p - (BYTE00064 *)m_memPtrArr[index];
	case 5: return (BYTE00128 *)p - (BYTE00128 *)m_memPtrArr[index];
	case 6: return (BYTE00256 *)p - (BYTE00256 *)m_memPtrArr[index];
	case 7: return (BYTE00512 *)p - (BYTE00512 *)m_memPtrArr[index];
	case 8: return (BYTE01024 *)p - (BYTE01024 *)m_memPtrArr[index];
	case 9: return (BYTE02048 *)p - (BYTE02048 *)m_memPtrArr[index];
	case 10: return (BYTE04096 *)p - (BYTE04096 *)m_memPtrArr[index];
	case 11: return (BYTE08192 *)p - (BYTE08192 *)m_memPtrArr[index];
	case 12: return (BYTE16384 *)p - (BYTE16384 *)m_memPtrArr[index];
	default: break;
	}
	return -1;
}

bool IndexMemPool::FindFreePtr(int & in1st, int & index, int & bit)
{
	for (; in1st < MAX_BUCKET_INDEX; ++in1st)
	{
		for (int i = 0; i < MAX_MEM_CHUNCK_CNT; ++i)
		{
			// -1 이면 다 쓰고 있는거
			if (m_memIndexPage[in1st][i] == -1)
			{
				continue;
			}
			else
			{				
				// 마스크 씌우기 시도
				if (false == setMask(&m_memIndexPage[in1st][i], bit))
				{
					// 안되면 그냥 다음 인덱스에서 하지 뭐
					continue;
				}
				else
				{
					// 오예! 성공함
					// 실제 index 값을 재계산 해서 넣어주자
					index = i;
					return true;
				}
			}
		}
	}
	return false;
}

bool IndexMemPool::setMask(int * p, int & bit)
{
	int bitVal{ 1 };

	// bit 어느 공간이 비었는지 체크하기
	for (int i = 0; i < BIT_SIZE_INT; ++i)
	{
		int ptrVal = *p;
		if (!(ptrVal & bitVal))
		{
			// 비어있으니 얼른 cas 후 return, 실패 리턴하면 그냥 넘겨야지 뭐
			bit = i;
			int markOn = ptrVal | bitVal;
			return CAS(p, ptrVal, markOn);
		}
		bitVal <<= 1;
	}

	return false;
}

void * IndexMemPool::Malloc(size_t s)
{
	// 할당 할 수 없는 범위 라면, nullptr 반환
	if (1 > s || s > 8192)
	{
		return nullptr;
	}

	// 얼만큼 크기의 버킷을 줄지 index 를 구하자
	int index1st = getIndexSize(s);
	size_t chunckSize = m_memSize[index1st];

	// 어디가 사용중이지 않는지 찾고, 쓰겠다고 표기하자
	int index2nd{ 0 }, indexBit{ 0 };
	if (false == FindFreePtr(index1st, index2nd, indexBit))
	{
		// 뭔가 할당을 못해줄 상황인것이다
		return nullptr;
	}

	size_t ptrDistance = ((index2nd * BIT_SIZE_INT) + indexBit) * chunckSize;
	// 이 위치에서, + size 만큼 이동해야 됨...
	// 그러나 size * 용량 크기 이렇게 이동해야된다.
	char * p = (char *)m_memPtrArr[index1st];
	return (void *)&p[ptrDistance];
}

void IndexMemPool::Free(void * p)
{
	if (nullptr == p)
	{
		return;
	}

	int index1st{ 0 };
	size_t pDis{ 0 };
	for (index1st = 0; index1st < MAX_BUCKET_INDEX; ++index1st)
	{
		// 해제할 포인터 위치에서, 할당 받은 앞 포인터를 각각 거리 비교 0 ~ 19999 이내 값이 나와야 한다
		pDis = getPtrDistance(p, index1st);
		if (MAX_MEM_CHUNCK_CNT > pDis && pDis >= 0)
		{
			break;
		}
	}

	// 이상한 위치의 포인터면 그냥 버리기
	if (MAX_BUCKET_INDEX == index1st)
	{
		return;
	}

	// 찾아서 indexPage 사용해제 해주기
	/// i 는 메모리 크기 구분하는 index, pDis 는 두번째 인덱스 구분용
	/// pDis / 32 = 두번째 인덱스
	/// pDis % 32 = 비트 위치

	size_t index2nd{ pDis / BIT_SIZE_INT };
	size_t indexBit{ pDis % BIT_SIZE_INT };
	int setBit{ 1 };
	setBit <<= indexBit;
	// ~setBit;	// 이렇게 해서 마스크 씌워버리면 간편할듯

	while (true)
	{
		int tempVal = m_memIndexPage[index1st][index2nd];
		int mask = tempVal & (~setBit);
		if (CAS(&m_memIndexPage[index1st][index2nd], tempVal, mask))
		{
			break;
		}
	}
}