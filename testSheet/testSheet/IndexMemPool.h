#pragma once
#include <Windows.h>
#include <atomic>
#include <iostream>

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
constexpr size_t MAX_BUCKET_INDEX{ 12 };
constexpr size_t MAX_MEM_CHUNCK_CNT{ 20000 };

namespace
{
	bool CAS(int * ptr, int oldVal, int newVal)
	{
		/*return std::atomic_compare_exchange_strong(
			reinterpret_cast<std::atomic_int *>(&ptr)
			, &oldVal
			, newVal);*/

		int temp = InterlockedCompareExchange(
			(volatile long *)ptr,
			(long)newVal,
			(long)oldVal
		);
		
		return temp == oldVal;
	}
}

class IndexMemPool
{
public:
	IndexMemPool();
	~IndexMemPool();


	void * Malloc(size_t s);

	void Free(void * p);

private:
	void Init();
	void Release();

	const size_t getChunckSize(const size_t & s);
	const int getIndexSize(const size_t & s);
	const long long getPtrDistance(const void * const p, const int & index);

	bool FindFreePtr(int & in1st, int & index, int & bit);
	bool setMask(int * p, int & bit);

	size_t m_memSize[MAX_BUCKET_INDEX]{ 0 };
	int m_memIndexPage[MAX_BUCKET_INDEX][MAX_MEM_CHUNCK_CNT]{ 0 };

	void * m_memPtrArr[MAX_BUCKET_INDEX]{ nullptr };
	size_t m_totalMemSize{ 0 };

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

