#include "define.h"
#include "FileOutRandSize.h"

#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <chrono>

using BYTE = unsigned char;

// 각 스레드별로 할당할 예정인 크기 값이, txt 파일을 읽어와서 여기에 담게 된다 - 구분은 스레드 id 값으로 접근하여 로컬로 관리
std::vector<std::vector<int>> g_needToAllocSizes;

// 각 스레드별 해제해야 하는 포인터를 갖게 되는데, 스레드 충돌 문제를 피하기 위해서, 각 스레드 별 id 당 4개의 각각 해제해야하는 목록을 추가로 갖는다.
//std::vector<std::vector<std::queue<void*>>> g_freePtrs;
BYTE * g_freeP[CPU_CORES][MAX_RETRY / CPU_CORES]{ 0 };

int getRndNum()
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, CPU_CORES - 1);
	return dist(mt);
}

void BenchMark(const size_t & id)
{
	const size_t other_id{ (id + 1) % CPU_CORES };
	size_t freeIndex{ 0 };

	for (size_t i = 0; i < (MAX_RETRY / CPU_CORES); i++)
	{
		BYTE * p = (BYTE *)malloc(g_needToAllocSizes[id][i]);
		*(int *)p = -1;
		g_freeP[id][i] = p;

		// 4회 마다 메모리 해제
		if (0 == (i % 6))
		{
			free(g_freeP[other_id][freeIndex]);
			g_freeP[other_id][freeIndex] = nullptr;
			++freeIndex;
		}
	}

	// 남은 메모리 한꺼번에 모두 해제
	for (size_t j = freeIndex; j < (MAX_RETRY / CPU_CORES); ++j)
	{
		free(g_freeP[other_id][j]);
	}
}


int main()
{

#if CurrType == TBB
	std::cout << "< TBB >\n";
#elif CurrType == tcMalloc
	std::cout << "< tcMalloc >\n";
#else
	std::cout << "< LFH >\n";
#endif

	//FileOutRandSize fo;
	//fo.FileOutRndSize();

	g_needToAllocSizes.reserve(CPU_CORES);
	for (size_t i = 0; i < CPU_CORES; ++i)
	{
		g_needToAllocSizes.emplace_back(std::vector<int>{});
		g_needToAllocSizes[i].reserve(MAX_RETRY / CPU_CORES);
	}

	std::ifstream inFile("Benchmark_for_size.txt");
	int size{ 0 };

	if (inFile.is_open())
	{
		// 초기화
		for (size_t j = 0; j < CPU_CORES; ++j)
		{
			for (size_t i = 0; i < MAX_RETRY / CPU_CORES; ++i)
			{
				inFile >> size;
				g_needToAllocSizes[j].emplace_back(size);
			}
		}


		size_t averageSum{ 0 };
		for (size_t reTry = 0; reTry < 10; ++reTry)
		{
			std::vector<std::thread> threadVector;
			threadVector.reserve(CPU_CORES);

			auto Tstart = std::chrono::high_resolution_clock::now();
			/// 측정구간 //////////////////////////////////////////////////////////////////////////////////////////////////////
			for (size_t i = 0; i < CPU_CORES; ++i) threadVector.emplace_back(BenchMark, i);
			for (auto & th : threadVector) th.join();
			/// 측정구간 //////////////////////////////////////////////////////////////////////////////////////////////////////
			auto Tend = std::chrono::high_resolution_clock::now();
			std::cout << "Duration Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(Tend - Tstart).count() << "ms\n";
			averageSum = averageSum + std::chrono::duration_cast<std::chrono::milliseconds>(Tend - Tstart).count();
		}
		std::cout << "Duration Average Time : " << averageSum / 10 << "ms\n\n";
	}
	else
	{
		std::cout << "FILE OPEN ERROR\n";
	}

	system("pause");
}