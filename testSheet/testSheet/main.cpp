#define ChooseType LFH

#define LFH 0
#define PPL 1
#define TBB 2
#define TCMALLOC 3
#define MYMEMPOOL 4

#if ChooseType == TCMALLOC
#pragma comment(lib, "libtcmalloc_minimal")
#pragma comment(linker, "/include:__tcmalloc")
#elif ChooseType == TBB
#include "tbb/tbbmalloc_proxy.h"
#endif


#include <iostream>
#include <random>
#include <vector>
#include <queue>
#include <algorithm>
#include <thread>
#include <chrono>
#include <ppl.h>
using namespace std;

constexpr int MAX_TRY{ 40000000 };
constexpr int MAX_INDEX{ 12 };	// 8100 까지만 테스트 하기 때문에 12
constexpr int threadNums{ 4 };

// 할당 비율 계산용
int allocPattern[40]{ 4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,64,64,64,64,128,128,256,256,512,512,1024,2048,4096,8192 };

// 확률 계산
// 4 Byte		= 20 %
// 8 Byte		= 20 %
// 16 Byte		= 15 %
// 32 Byte		= 10 %
// 64 Byte		= 10 %
// 128 Byte		= 5  %
// 256 Byte		= 5  %
// 512 Byte		= 5  %
// 1024 Byte	= 2.5%
// 2048 Byte	= 2.5%
// 4096 Byte	= 2.5%
// 8192 Byte	= 2.5%

vector<int> allocvec;
#if (ChooseType == MYMEMPOOL)
#include "IndexMemPool.h"
IndexMemPool g_myMemPool;
#endif

void benchMarkAllAlloc(int th_id)
{
	size_t reTry = (allocvec.size() / threadNums);
	vector<void *> localAllocPtr;
	localAllocPtr.reserve(reTry);

	// alloc
	for (int i = 0; i < reTry; ++i)
	{
#if ((ChooseType == LFH) || (ChooseType == TCMALLOC) || (ChooseType == TBB))
		localAllocPtr.emplace_back(malloc(allocvec[i + th_id]));
#elif (ChooseType == PPL)
		localAllocPtr.emplace_back(concurrency::Alloc(allocvec[i + th_id]));
#elif (ChooseType == MYMEMPOOL)
		localAllocPtr.emplace_back(g_myMemPool.Malloc(allocvec[i + th_id]));
#endif
		*(int *)localAllocPtr[i] = -1;
	}

	// free
	for (const auto & d : localAllocPtr)
	{
#if ((ChooseType == LFH) || (ChooseType == TCMALLOC) || (ChooseType == TBB))
		free(d);
#elif (ChooseType == PPL)
		concurrency::Free(d);
#elif (ChooseType == MYMEMPOOL)
		g_myMemPool.Free(d);
#endif
	}
}

// int alloc
void benchMarkIntAlloc(int th_id)
{
	int reTry = (240000 / threadNums);
	vector<void *> localAllocPtr;
	localAllocPtr.reserve(reTry);

	for (int i = 0; i < reTry; ++i)
	{
#if ((ChooseType == LFH) || (ChooseType == TCMALLOC) || (ChooseType == TBB))
		localAllocPtr.emplace_back(malloc(sizeof(int)));
#elif (ChooseType == PPL)
		localAllocPtr.emplace_back(concurrency::Alloc(sizeof(int)));
#elif (ChooseType == MYMEMPOOL)
		localAllocPtr.emplace_back(g_myMemPool.Malloc(sizeof(int)));
#endif
		*(int *)localAllocPtr[i] = -1;
	}

	// free
	for (const auto & d : localAllocPtr)
	{
#if ((ChooseType == LFH) || (ChooseType == TCMALLOC) || (ChooseType == TBB))
		free(d);
#elif (ChooseType == PPL)
		concurrency::Free(d);
#elif (ChooseType == MYMEMPOOL)
		g_myMemPool.Free(d);
#endif
	}
}

// real alloc
void benchMarkReal(int th_id)
{
	int reTry = MAX_TRY / threadNums;
	random_device rd;
	mt19937 mt(rd());
	uniform_int_distribution<int> dist(0, 39);

	queue<void *> localAllocPtr[2];

	for (int i = 0; i < reTry; ++i)
	{
#if (ChooseType == LFH) || (ChooseType == TBB) || (ChooseType == TCMALLOC)
		localAllocPtr[dist(mt) & 1].push(malloc(allocPattern[dist(mt)]));
#elif (ChooseType == PPL)
		localAllocPtr[dist(mt) & 1].push(concurrency::Alloc(allocPattern[dist(mt)]));
#elif (ChooseType == MYMEMPOOL)
		localAllocPtr[dist(mt) & 1].push(g_myMemPool.Malloc(allocPattern[dist(mt)]));
#endif
		if (!(i % 4))
		{
			if (localAllocPtr[0].size())
			{
#if (ChooseType == LFH) || (ChooseType == TBB) || (ChooseType == TCMALLOC)
				free(localAllocPtr[0].front());
#elif (ChooseType == PPL)
				concurrency::Free(localAllocPtr[0].front());
#elif (ChooseType == MYMEMPOOL)
				g_myMemPool.Free(localAllocPtr[0].front());
#endif
				localAllocPtr[0].pop();
			}
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		while (localAllocPtr[i].size())
		{
#if (ChooseType == LFH) || (ChooseType == TBB) || (ChooseType == TCMALLOC)
			free(localAllocPtr[i].front());
#elif (ChooseType == PPL)
			concurrency::Free(localAllocPtr[i].front());
#elif (ChooseType == MYMEMPOOL)
			g_myMemPool.Free(localAllocPtr[i].front());
#endif
			localAllocPtr[i].pop();
		}
	}
}

int main()
{
#if (ChooseType == LFH)
	cout << "< LFH >\n";
#elif (ChooseType == PPL)
	cout << "< PPL >\n";
#elif (ChooseType == TBB)
	cout << "< TBB >\n";
#elif (ChooseType == TCMALLOC)
	cout << "< TCMALLOC >\n";
#elif (ChooseType == MYMEMPOOL)
	cout << "< MY_MEM_POOL >\n";
#endif
	// 전체 할당 후 해제 방식
	int allocSize{ 4 };
	for (int j = 0; j < MAX_INDEX; ++j)
	{
		for (int i = 0; i < 20000; ++i)
		{
			allocvec.emplace_back(allocSize);
		}
		allocSize *= 2;
	}

	random_shuffle(allocvec.begin(), allocvec.end());

	thread * threads[threadNums];

	auto Tstart = chrono::high_resolution_clock::now();
	auto Tend = chrono::high_resolution_clock::now();
	int averTimes{ 5 };
	size_t averageSum{ 0 };

	// Full Alloc
	for (int i = 0; i < averTimes; ++i)
	{
		Tstart = chrono::high_resolution_clock::now();
		for (int i = 0; i < threadNums; ++i) threads[i] = new thread{ benchMarkAllAlloc, i };
		for (int i = 0; i < threadNums; ++i) threads[i]->join();
		Tend = chrono::high_resolution_clock::now();
		cout << "Duration Time : " << chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count() << "ms\t- Full Alloc\n";
		averageSum = averageSum + chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count();
		for (int i = 0; i < threadNums; ++i) delete threads[i];
	}
	cout << "Duration Average Time : " << averageSum / averTimes << "ms\n\n";
	averageSum = 0;

	// int Alloc
	for (int i = 0; i < averTimes; ++i)
	{
		Tstart = chrono::high_resolution_clock::now();
		for (int i = 0; i < threadNums; ++i) threads[i] = new thread{ benchMarkIntAlloc, i };
		for (int i = 0; i < threadNums; ++i) threads[i]->join();
		Tend = chrono::high_resolution_clock::now();
		cout << "Duration Time : " << chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count() << "ms\t- int Alloc\n";
		averageSum = averageSum + chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count();
		for (int i = 0; i < threadNums; ++i) delete threads[i];
	}
	cout << "Duration Average Time : " << averageSum / averTimes << "ms\n\n";
	averageSum = 0;

#if ChooseType != MYMEMPOOL
	// real Alloc
	for (int i = 0; i < averTimes; ++i)
	{
		Tstart = chrono::high_resolution_clock::now();
		for (int i = 0; i < threadNums; ++i) threads[i] = new thread{ benchMarkReal, i };
		for (int i = 0; i < threadNums; ++i) threads[i]->join();
		Tend = chrono::high_resolution_clock::now();
		cout << "Duration Time : " << chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count() << "ms\t- Real Alloc\n";
		averageSum = averageSum + chrono::duration_cast<chrono::milliseconds>(Tend - Tstart).count();
		for (int i = 0; i < threadNums; ++i) delete threads[i];
	}
	cout << "Duration Average Time : " << averageSum / averTimes << "ms\n\n";
	averageSum = 0;
#endif
}