#include "idatetime.h"
#if defined(_WIN64) || defined(_WIN32) || defined(_MSC_VER)
#include <windows.h>
#elif defined(__linux) || defined(__linux__)
#include <sys/time.h>
#include <time.h>
#endif

uint64_i getdida()
{
#if defined(_WIN64) || defined(_WIN32) || defined(_MSC_VER)
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	double f = (double)freq.QuadPart;
	QueryPerformanceCounter(&freq);
	double s = freq.QuadPart;
	return (s / f) * 1000 * 1000;
#elif defined(__linux) || defined(__linux__)
	struct timespec dida;
	clock_gettime(CLOCK_MONOTONIC, &dida);
	return (dida.tv_sec * 1000000) + (dida.tv_nsec / 1000);

#else
	printf("0ULLL\n");
	return 0ULL; /**unsupported OS.*/
#endif
}

uint64_i getdida_msec()
{
	return getdida() / 1000ULL;
}

void igettimeofday(struct timeval *tv)
{
#if defined(WINDOWS) && !defined(__MINGW_H)
	FILETIME ft;
	u_int64_t tim;
	GetSystemTimeAsFileTime(&ft);
	tim = filetime_to_unix_epoch(&ft);
	tv->tv_sec = (long)(tim / 1000000ULL);
	tv->tv_usec = (long)(tim % 1000000ULL);
#else
	gettimeofday(tv, NULL);
#endif
}

uint64_i getts()
{
	struct timeval tv;
	igettimeofday(&tv);
	uint64_i ts = tv.tv_sec;
	ts *= 1000000ULL;
	ts += tv.tv_usec;
	return ts;
}

uint64_i getts_msec()
{
	return getts() / 1000ULL;
}

uint32_i getclock32()
{
	uint64_i isec = getdida_msec() / 1000ULL;
	return (uint32_i)(isec & 0xFFFFFFFFul);
}
