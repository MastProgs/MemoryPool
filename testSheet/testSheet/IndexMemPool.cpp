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
			// -1 �̸� �� ���� �ִ°�
			if (m_memIndexPage[in1st][i] == -1)
			{
				continue;
			}
			else
			{				
				// ����ũ ����� �õ�
				if (false == setMask(&m_memIndexPage[in1st][i], bit))
				{
					// �ȵǸ� �׳� ���� �ε������� ���� ��
					continue;
				}
				else
				{
					// ����! ������
					// ���� index ���� ���� �ؼ� �־�����
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

	// bit ��� ������ ������� üũ�ϱ�
	for (int i = 0; i < BIT_SIZE_INT; ++i)
	{
		int ptrVal = *p;
		if (!(ptrVal & bitVal))
		{
			// ��������� �� cas �� return, ���� �����ϸ� �׳� �Ѱܾ��� ��
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
	// �Ҵ� �� �� ���� ���� ���, nullptr ��ȯ
	if (1 > s || s > 8192)
	{
		return nullptr;
	}

	// ��ŭ ũ���� ��Ŷ�� ���� index �� ������
	int index1st = getIndexSize(s);
	size_t chunckSize = m_memSize[index1st];

	// ��� ��������� �ʴ��� ã��, ���ڴٰ� ǥ������
	int index2nd{ 0 }, indexBit{ 0 };
	if (false == FindFreePtr(index1st, index2nd, indexBit))
	{
		// ���� �Ҵ��� ������ ��Ȳ�ΰ��̴�
		return nullptr;
	}

	size_t ptrDistance = ((index2nd * BIT_SIZE_INT) + indexBit) * chunckSize;
	// �� ��ġ����, + size ��ŭ �̵��ؾ� ��...
	// �׷��� size * �뷮 ũ�� �̷��� �̵��ؾߵȴ�.
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
		// ������ ������ ��ġ����, �Ҵ� ���� �� �����͸� ���� �Ÿ� �� 0 ~ 19999 �̳� ���� ���;� �Ѵ�
		pDis = getPtrDistance(p, index1st);
		if (MAX_MEM_CHUNCK_CNT > pDis && pDis >= 0)
		{
			break;
		}
	}

	// �̻��� ��ġ�� �����͸� �׳� ������
	if (MAX_BUCKET_INDEX == index1st)
	{
		return;
	}

	// ã�Ƽ� indexPage ������� ���ֱ�
	/// i �� �޸� ũ�� �����ϴ� index, pDis �� �ι�° �ε��� ���п�
	/// pDis / 32 = �ι�° �ε���
	/// pDis % 32 = ��Ʈ ��ġ

	size_t index2nd{ pDis / BIT_SIZE_INT };
	size_t indexBit{ pDis % BIT_SIZE_INT };
	int setBit{ 1 };
	setBit <<= indexBit;
	// ~setBit;	// �̷��� �ؼ� ����ũ ���������� �����ҵ�

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