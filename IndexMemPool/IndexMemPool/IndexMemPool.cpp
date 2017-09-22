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
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		m_memSize[i] = val;
		m_memPtrArr[i] = malloc(val * MAX_MEM_CHUNCK_CNT);
		val <<= 1;
	}
}

void IndexMemPool::Release()
{
	for (int i = 0; i < MAX_BUCKET_INDEX; ++i)
	{
		free(m_memPtrArr[i]);
	}
}

const int IndexMemPool::getBucketSize(const size_t & s)
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

const int IndexMemPool::getPtrDistance(const void const * p, const int & index)
{
	switch (index)
	{
	case 0: return (BYTE00004 *)p - m_memPtrArr[index];
	case 1: return (BYTE00008 *)p - m_memPtrArr[index];
	case 2: return (BYTE00016 *)p - m_memPtrArr[index];
	case 3: return (BYTE00032 *)p - m_memPtrArr[index];
	case 4: return (BYTE00064 *)p - m_memPtrArr[index];
	case 5: return (BYTE00128 *)p - m_memPtrArr[index];
	case 6: return (BYTE00256 *)p - m_memPtrArr[index];
	case 7: return (BYTE00512 *)p - m_memPtrArr[index];
	case 8: return (BYTE01024 *)p - m_memPtrArr[index];
	case 9: return (BYTE02048 *)p - m_memPtrArr[index];
	case 10: return (BYTE04096 *)p - m_memPtrArr[index];
	case 11: return (BYTE08192 *)p - m_memPtrArr[index];
	case 12: return (BYTE16384 *)p - m_memPtrArr[index];
	default:
		break;
	}
	return -1;
}

bool IndexMemPool::FindFreePtr(const int & in1st, int & index, int & bit)
{
	for (int j = 0; j < 2; ++j)
	{
		long long * ptr = (long long *)m_memIndexPage[in1st];
		for (int i = 0; i < MAX_BUCKET_INDEX / 2; ++i)
		{
			// -1 이면 다 쓰고 있는거
			if (ptr[i] == -1)
			{
				continue;
			}
			else
			{
				// -1 이 아니면 빈 공간이 어딘가에 있다
				int * pTemp = (int *)ptr;
				if (*pTemp == -1)
				{
					// 뒷칸 비어있음
					++pTemp;
					setMask(pTemp);
				}
				else
				{
					// 앞칸 비어있음
					setMask(pTemp);
				}
			}
		}
	}
	return false;
}

bool IndexMemPool::setMask(int * p)
{
	int bitVal{ 1 };

	// bit 어느 공간이 비었는지 체크하기
	for (int i = 0; i < BIT_SIZE_INT; ++i)
	{
		int ptrVal = *p;
		if (!(ptrVal & bitVal))
		{
			// 비어있으니 얼른 cas 후 return, 실패 리턴하면 그냥 넘겨야지 뭐
		}
		bitVal <<= 1;
	}

	return false;
}
