#pragma once

#include <Windows.h>
//#include <mutex>

#define LLval(ptr)					(*(long long *)(ptr))
#define ZeroMem(dest, size)			memset(dest, 0, size)

using DS = struct DebugStruct
{
	long long a;
	int b;
};

constexpr long long Tag_size{ 8LL };
constexpr long long Max_mem_size{ 10000000000LL };		// 10 GBytes
constexpr long long Max_alloc_size{ Max_mem_size - (Tag_size * 2) };

namespace TAG
{
	constexpr long long Mask{ 0x7FFFFFFFFFFFFFFFLL };
	constexpr long long isUse{ 0x8000000000000000LL };
}

class MyPool2
{
public:
	MyPool2();
	~MyPool2();

	void* Malloc(const size_t & s);

	void Free(void * p);

private:
	char * m_pMain{ nullptr };
	//char * m_pIter{ nullptr };
	char * m_pEnd{ nullptr };

	//std::mutex m_L;
};

namespace
{
	bool CAS(void * pWantToChange, long long oldVal, long long newVal)
	{
		InterlockedCompareExchange64(
			(volatile LONG64 *)pWantToChange
			, newVal
			, oldVal);

		return newVal == *(long long *)pWantToChange;

		/*return std::atomic_compare_exchange_strong(
		reinterpret_cast<std::atomic_int *>(&pWantToChange)
		, &oldVal
		, newVal);*/
	}
}