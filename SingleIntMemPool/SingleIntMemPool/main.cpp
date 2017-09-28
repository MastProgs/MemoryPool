
#include "SingleIntMemPool.h"

void f(int * p, int num)
{
	if (*p != num)
	{
		std::cout << "Not Same Data\n";
	}
}

int main()
{
	SingleIntMemPool mp;

	for (int i = 0; i < 1000; ++i)
	{
		int * p1 = mp.Malloc(1);
		int * p2 = mp.Malloc((rand() % 5) + 1);
		int * p3 = mp.Malloc(1);
		int * p4 = mp.Malloc(5);

		*p1 = -1;
		*p2 = -2;
		*p3 = -3;
		p4[0] = -4;

		f(p1, -1);
		f(p2, -2);
		f(p3, -3);
		f(&p4[0], -4);

		mp.Free(p1);
		mp.Free(p2);
		mp.Free(p3);
		mp.Free(p4);
	}
}