#ifndef __SHARED_UTILS_TIMER_H__
#define __SHARED_UTILS_TIMER_H__

#include <ctime>

#ifdef WIN32
#else

#include <netinet/in.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#endif

#include "../Net/Fd.h"
#include "../Serialization/ByteBuffer.h"

class Timer
{
public:
    Timer(void);
    ~Timer(void);

public:
    static uint32_t GetTimeSeconds(void);
    static uint64_t GetMilliSeconds(void);
    static uint64_t GetMicroSeconds(void);
    static uint64_t GetTimeDifference(uint64_t currentTime, uint64_t previousTime);

public:
    static const uint32_t MONTHS;
    static const uint32_t HOURS;
    static const uint32_t MINUTES;
    static const uint32_t SECONDS;
    static const uint32_t MILLI_SECONDS;
    static const uint32_t MICRO_SECONDS;
    static const uint32_t NANO_SECONDS;

protected:
#ifdef WIN32
    static const uint64_t UTC_TIME;
#endif
};

#endif 
