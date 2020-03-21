#pragma once

#include "Common.h"
#include <cstdarg>
#include <cstring>
#include <ctime>

namespace Prismata
{
    namespace Assert
    {
        const std::string currentDateTime();

        void ReportFailure(const char * condition, const char * file, int line, const char * msg, ...);
    }
}

#define PRISMATA_ASSERT_ALL

#ifdef PRISMATA_ASSERT_ALL

    #define PRISMATA_ASSERT(cond, msg, ...) \
        do \
        { \
            if (!(cond)) \
            { \
                Prismata::Assert::ReportFailure(#cond, __FILE__, __LINE__, (msg), ##__VA_ARGS__); \
            } \
        } while(0)

#else
    #define PRISMATA_ASSERT(cond, msg, ...) 
#endif
