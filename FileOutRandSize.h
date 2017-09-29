#pragma once
#include "define.h"
#include <random>
#include <fstream>

constexpr int MAX_INDEX_OF_ALLOC_CHECK{ 40 };

class FileOutRandSize
{
	// 할당 비율 계산용
	const int m_allocSize[MAX_INDEX_OF_ALLOC_CHECK]{ 4,4,4,4,4,4,4,4,8,8,8,8,8,8,8,8,16,16,16,16,16,16,32,32,32,32,64,64,64,64,128,128,256,256,512,512,1024,2048,4096,8192 };

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
	// 16384 Byte	= 0  %

public:
	FileOutRandSize();
	~FileOutRandSize();

	bool ofSize() const;
private:
	int getRndSize() const;
};

