/**
 * @file   Time.cpp
 * @author Xuzhou
 */

#include "../Net/Fd.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include "Timer.h"
#include "../Serialization/ByteBuffer.h"

const uint32_t Timer::MONTHS  = 30;
const uint32_t Timer::HOURS   = 24;
const uint32_t Timer::MINUTES = 60;
const uint32_t Timer::SECONDS = 60;
const uint32_t Timer::MILLI_SECONDS = 1000;
const uint32_t Timer::MICRO_SECONDS = 1000000;
const uint32_t Timer::NANO_SECONDS  = 10;
#ifdef WIN32
const uint64_t Timer::UTC_TIME      = 0x19db1ded53e8000;
#endif

Timer::Timer()
{
}

Timer::~Timer()
{
}

uint32_t 
Timer::GetTimeSeconds(void)
{
    return (uint32_t)(Timer::GetMicroSeconds() / Timer::MICRO_SECONDS);
}

uint64_t 
Timer::GetMilliSeconds(void)
{
    return Timer::GetMicroSeconds() / Timer::MILLI_SECONDS;
}

uint64_t 
Timer::GetMicroSeconds(void)
{
#ifdef WIN32
    ::ULARGE_INTEGER time;
    ::GetSystemTimeAsFileTime((FILETIME*)&time);

    return (time.QuadPart - Timer::UTC_TIME) / Timer::NANO_SECONDS;
#else
    ::timeval time;
    ::gettimeofday(&time, 0);
    
    return time.tv_sec * Timer::MICRO_SECONDS + time.tv_usec;
#endif
}

uint64_t 
Timer::GetTimeDifference(uint64_t currentTime, uint64_t previousTime)
{
    return currentTime - previousTime;
}

