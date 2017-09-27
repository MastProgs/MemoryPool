
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

template<typename T>
bool EXPECT_EQ(T Expected, T Value)
{
	if (Expected != Value)
	{
		std::cout << "Test failed : Expected =" << Expected << " Value = " << Value << std::endl;
		return false;
	}
	return true;
}

void SingleThreadTest()
{
	int * p1 = (int *)mp.Malloc(sizeof(int));
	int * p2 = (int *)mp.Malloc(sizeof(int));
	int * p3 = (int *)mp.Malloc(sizeof(int));

	*p1 = -1;
	*p2 = -2;
	*p3 = -3;

	EXPECT_EQ(-1, *p1);
	EXPECT_EQ(-2, *p2);
	EXPECT_EQ(-3, *p3);

	mp.Free(p1);
	mp.Free(p2);
	mp.Free(p3);
}

int main()
{
	SingleThreadTest();


}