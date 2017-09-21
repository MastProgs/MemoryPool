#pragma once
#include <chrono>

class TimeCheck
{
public:
	TimeCheck();
	~TimeCheck();

	void TimeCheckBeg() { m_tpBeg = std::chrono::high_resolution_clock::now(); };
	void TimeCheckEnd() { m_tpEnd = std::chrono::high_resolution_clock::now(); };
	long long TakenTime() { return std::chrono::duration_cast<std::chrono::milliseconds>(m_tpEnd - m_tpBeg).count(); };

private:
	std::chrono::high_resolution_clock::time_point m_tpBeg;
	std::chrono::high_resolution_clock::time_point m_tpEnd;
};

