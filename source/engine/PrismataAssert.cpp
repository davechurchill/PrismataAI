#include "PrismataAssert.h"

using namespace Prismata;

namespace Prismata
{
namespace Assert
{
    const std::string currentDateTime() 
    {
        time_t     now = time(0);
        struct tm  tstruct;
        char       buf[80];
        tstruct = *localtime(&now);
        strftime(buf, sizeof(buf), "%Y-%m-%d_%X", &tstruct);

        for (size_t i(0); i < strlen(buf); ++i)
        {
            if (buf[i] == ':')
            {
                buf[i] = '-';
            }
        }

        return buf;
    }

    void ReportFailure(const char * condition, const char * file, int line, const char * msg, ...)
    {
        std::cout << "Assertion thrown!\n";

        char messageBuffer[4096] = "";
        if (msg != NULL)
        {
            va_list args;
            va_start(args, msg);
            vsprintf(messageBuffer, msg, args);
            va_end(args);
        }

        std::stringstream ss;
        ss                                              << std::endl;
        ss << "!Assert:   " << condition                << std::endl;
        ss << "File:      " << file                     << std::endl;
        ss << "Message:   " << messageBuffer            << std::endl;
        ss << "Line:      " << line                     << std::endl;
        
        fprintf(stderr, "%s", ss.str().c_str());  
    }
}
}

