#pragma once

#include <cstddef>

#ifdef WIN32   
 #include <windows.h>

 #undef __wchar_t
#else          
 #include <sys/time.h>
#endif

namespace Prismata
{
class Timer
{
  double startTimeInMicroSec;                 
    double endTimeInMicroSec;                  
    int    stopped;                             
 #ifdef WIN32
  LARGE_INTEGER frequency;                    
  LARGE_INTEGER startCount;                   
  LARGE_INTEGER endCount;                     
 #else
  timeval startCount;                        
  timeval endCount;                          
 #endif

public:

 Timer();
 
    ~Timer();

    void start();
 
    void stop();
 
    double getElapsedTimeInMicroSec();
    double getElapsedTimeInMilliSec();
    double getElapsedTimeInSec();
    double getElapsedTime();
};
}