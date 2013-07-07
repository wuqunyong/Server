/*
 Copyright (c) 2007-2011 iMatix Corporation
 Copyright (c) 2007-2011 Other contributors as noted in the AUTHORS file

 This file is part of 0MQ.

 0MQ is free software; you can redistribute it and/or modify it under
 the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 0MQ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Clock.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#ifdef WIN32    
#include <time.h>  
#else
#include <unistd.h>
#include <sys/time.h>
#endif


FREEZE_NET::Clock::Clock(void)
{
}

FREEZE_NET::Clock::~Clock(void)
{
}

uint64_t FREEZE_NET::Clock::NowMS(void)
{
#ifdef WIN32
	time_t timer;
	time(&timer); 

	return timer*1000;
#else
	struct timeval tv;
	int rc = gettimeofday(&tv, NULL);
	assert (rc == 0);
	return ((tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec) / 1000);
#endif
}

