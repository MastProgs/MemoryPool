
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "IndexMemPool.h"

class TimeCheck
{
public:
	TimeCheck() = default;
	~TimeCheck() = default;

	void TimeCheckBeg() { m_tpBeg = std::chrono::high_resolution_clock::now(); };
	void TimeCheckEnd() { m_tpEnd = std::chrono::high_resolution_clock::now(); };
	long long TakenTime() { return std::chrono::duration_cast<std::chrono::milliseconds>(m_tpEnd - m_tpBeg).count(); };

private:
	std::chrono::high_resolution_clock::time_point m_tpBeg;
	std::chrono::high_resolution_clock::time_point m_tpEnd;
};

IndexMemPool mp;

void f()
{
	for (int i = 0; i < 10000000; ++i)
	{
		int * p = (int *)mp.Malloc(sizeof(int));
		*p = -1;
		mp.Free(p);
	}
}

int main()
{
	TimeCheck time;
	std::vector<std::thread> threadVector;

	time.TimeCheckBeg();
	for (int i = 0; i < 4; ++i)
	{
		threadVector.emplace_back(std::thread{ f });
	}
	for (auto & d : threadVector)
	{
		d.join();
	}
	time.TimeCheckEnd();

	std::cout << time.TakenTime() << " ms\n";
}