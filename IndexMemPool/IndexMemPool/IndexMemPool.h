#pragma once
#include <atomic>

using BYTE00004 = int;
using BYTE00008 = long long;
using BYTE00016 = struct M16 { BYTE00008 a; BYTE00008 b; };
using BYTE00032 = struct M32 { BYTE00016 a; BYTE00016 b; };
using BYTE00064 = struct M64 { BYTE00032 a; BYTE00032 b; };
using BYTE00128 = struct M128 { BYTE00064 a; BYTE00064 b; };
using BYTE00256 = struct M256 {	BYTE00128 a; BYTE00128 b; };
using BYTE00512 = struct M512 {	BYTE00256 a; BYTE00256 b; };
using BYTE01024 = struct M1024 { BYTE00512 a; BYTE00512 b; };
using BYTE02048 = struct M2048 { BYTE01024 a; BYTE01024 b; };
using BYTE04096 = struct M4096 { BYTE02048 a; BYTE02048 b; };
using BYTE08192 = struct M8192 { BYTE04096 a; BYTE04096 b; };
using BYTE16384 = struct M16384 { BYTE08192 a; BYTE08192 b; };

constexpr size_t BIT_SIZE_INT{ 32 };

constexpr size_t MIN_MEM_SIZE{ 4 };
constexpr size_t MAX_BUCKET_INDEX{ 13 };
constexpr size_t MAX_MEM_CHUNCK_CNT{ 5000 };

namespace
{
	bool CAS(void *ptr, int oldVal, int newVal)
	{
		return std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic_int *>(&ptr)
			, &oldVal
			, newVal);
	}
}

class IndexMemPool
{
public:
	IndexMemPool();
	~IndexMemPool();


	void * Malloc(size_t s)
	{
		// 할당 할 수 없는 범위 라면, nullptr 반환
		if (1 > s || s > 16384)
		{
			return nullptr;
		}

		// 얼만큼 크기의 버킷을 줄지 index 를 구하자
		int index1st = getIndexSize(s);

		// 어디가 사용중이지 않는지 찾고, 쓰겠다고 표기하자
		int index2nd{ 0 }, indexBit{ 0 };
		if (false == FindFreePtr(index1st, index2nd, indexBit))
		{
			// 뭔가 할당을 못해줄 상황인것이다
			return nullptr;
		}

		int calculateChunkIndex{ (index2nd * BIT_SIZE_INT) + indexBit };
		int ptrDistance = (index2nd * m_memSize[index1st]) + indexBit;
		char * p = (char *)m_memPtrArr[index1st];
		return (void *)p[ptrDistance];
	}

	void Free(void * p)
	{
		int index1st{ 0 }, pDis{ 0 };
		for (index1st = 0; index1st < MAX_BUCKET_INDEX; ++index1st)
		{
			// 해제할 포인터 위치에서, 할당 받은 앞 포인터를 각각 거리 비교 0 ~ 4999 이내 값이 나와야 한다
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

		int index2nd{ pDis / BIT_SIZE_INT };
		int indexBit{ pDis % BIT_SIZE_INT };
		int setBit{ 1 };	
		setBit <<= indexBit;
		// ~setBit;	// 이렇게 해서 마스크 씌워버리면 간편할듯

		while (true)
		{
			int tempVal = m_memIndexPage[index1st][index2nd];
			if (CAS(&m_memIndexPage[index1st][index2nd], tempVal, tempVal & (~setBit)))
			{
				break;
			}
		}
	}

private:
	void Init();
	void Release();

	const int getBucketSize(const size_t & s);
	const int getIndexSize(const size_t & s);
	const int getPtrDistance(const void const * p, const int & index);

	bool FindFreePtr(const int & in1st, int & index, int & bit);
	bool setMask(int * p);

	size_t m_memSize[MAX_BUCKET_INDEX]{ 0 };
	int m_memIndexPage[MAX_BUCKET_INDEX][MAX_MEM_CHUNCK_CNT]{ 0 };

	void * m_memPtrArr[MAX_BUCKET_INDEX]{ nullptr };

	/*BYTE00004 * pMain4		{ nullptr };
	BYTE00008 * pMain8		{ nullptr };
	BYTE00016 * pMain16		{ nullptr };
	BYTE00032 * pMain32		{ nullptr };
	BYTE00064 * pMain64		{ nullptr };
	BYTE00128 * pMain128	{ nullptr };
	BYTE00256 * pMain256	{ nullptr };
	BYTE00512 * pMain512	{ nullptr };
	BYTE01024 * pMain1024	{ nullptr };
	BYTE02048 * pMain2048	{ nullptr };
	BYTE04096 * pMain4096	{ nullptr };
	BYTE08192 * pMain8192	{ nullptr };
	BYTE16384 * pMain16384	{ nullptr };*/
};

