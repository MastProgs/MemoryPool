#include "FileOutRandSize.h"



FileOutRandSize::FileOutRandSize()
{
}


FileOutRandSize::~FileOutRandSize()
{
}

int FileOutRandSize::getRndSize() const
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<int> dist(0, MAX_INDEX_OF_ALLOC_CHECK - 1);

	return m_allocSize[dist(mt)];
}

bool FileOutRandSize::ofSize() const
{
	std::ofstream outf("Benchmark_for_size.txt");

	for (size_t i = 0; i < MAX_RETRY; ++i)
	{
		outf << getRndSize() << " ";
	}
	outf.close();

	return true;
}
