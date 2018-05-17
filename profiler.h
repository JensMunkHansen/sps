/*
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright 2017 Jens Munk Hansen
 */
#ifndef SPS_PROFILER_H_
#define SPS_PROFILER_H_

#include <sps/cenv.h>

#ifdef _MSC_VER
# include <windows.h>
#else
# include <time.h>
# include <sys/time.h>
# include <sys/resource.h>
#endif

namespace sps {
class profiler {
 public:
#ifdef _MSC_VER
  static double time(void)
  {
    static __THREAD double freq = 0.0;
    if (freq == 0.0) {
      LARGE_INTEGER llDiv;
      QueryPerformanceFrequency(&llDiv);
      freq = double(llDiv.QuadPart);
    }
    LARGE_INTEGER ll;
    QueryPerformanceCounter(&ll);
    return double(ll.QuadPart) / freq;
  }
#else
  static double time(void)
  {

    SPS_REGISTER double sec;
    SPS_REGISTER double usec;

#if 1
    struct timespec tspec;
    clock_gettime(CLOCK_MONOTONIC, &tspec);
    sec = (double) tspec.tv_sec;
    usec = ((double) tspec.tv_nsec) / 1000000000.0;
#else
    static __THREAD struct rusage foo;
    getrusage (RUSAGE_SELF, &foo); // process values
    sec = (double) foo.ru_utime.tv_sec;
    usec = ((double) foo.ru_utime.tv_usec) / 1000000.0;
#endif
    return (sec + usec);
  }
#endif
};
}  // namespace sps

#endif  // SPS_PROFILER_H_

/*

clock() from <time.h> (20ms or 10ms resolution?)

gettimeofday() from Posix <sys/time.h> (microseconds)

clock_gettime() on Posix (nanoseconds?) // CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID,
CLOCK_MONOTONIC, CLOCK_REALTIME
*/
