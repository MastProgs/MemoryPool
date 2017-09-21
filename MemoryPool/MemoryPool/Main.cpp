
//#include "TBBalloc.h"
#include "MyPool2.h"
#include "TimeCheck.h"
#include <iostream>

int main()
{
	TimeCheck time;
	//MyFreeList freelist{ sizeof(int) };
	MyPool2 mp;

	time.TimeCheckBeg();
	for (int i = 0; i < 10000000; ++i)
	{
		int * p = (int *)mp.Malloc(sizeof(int));
		*p = -1;
		mp.Free(p);
	}
	time.TimeCheckEnd();

	std::cout << time.TakenTime() << " ms\n";
}