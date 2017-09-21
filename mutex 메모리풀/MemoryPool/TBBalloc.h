#pragma once

#ifdef TCALLOC
#undef TCALLOC
#endif // TCALLOC


#ifndef TBBALLOC
#define TBBALLOC

#include "tbb/tbbmalloc_proxy.h"
#include <iostream>

class TBBalloc
{
public:
	TBBalloc() {};
	~TBBalloc() {};

	void * tbbAlloc(size_t s) { return malloc(s); }
	void tbbFree(void * p) { free(p); }
};

#undef TBBALLOC
#endif // !TBBALLOC