#pragma once
#define CurrType tcMalloc

#define LFH 0
#define TBB 1
#define tcMalloc 2

constexpr int MAX_RETRY{ 32000000 };
constexpr int CPU_CORES{ 4 };

#if CurrType == TBB
#include "tbb/tbbmalloc_proxy.h"
#elif CurrType == tcMalloc
#pragma comment(lib, "libtcmalloc_minimal")
#pragma comment(linker, "/include:__tcmalloc")
#endif