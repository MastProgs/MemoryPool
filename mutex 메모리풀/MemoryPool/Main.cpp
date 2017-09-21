
//#include "TBBalloc.h"
#include "MyPool2.h"
#include "TimeCheck.h"
#include <iostream>

#include <thread>
#include <vector>

MyPool2 mp;

void f()
{
	for (int i = 0; i < 10000000 / 4; ++i)
	{
		int * p = (int *)mp.Malloc(sizeof(int));
		if (p == nullptr)
		{
			--i;
			continue;
		}
		*p = -1;
		mp.Free(p);
	}
}

int main()
{
	TimeCheck time;
	//MyFreeList freelist{ sizeof(int) };

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