#include "FileUtils.h"
#include "PrismataAssert.h"
#include <stdio.h>

namespace Prismata
{

namespace FileUtils
{
    std::string ReadFile(const std::string & filename)
    {
        std::stringstream ss;

        FILE *file = fopen ( filename.c_str(), "r" );
        if ( file != NULL )
        {
            char line [ 4096 ]; /* or other suitable maximum line size */
            while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
            {
                ss << line;
            }
            fclose ( file );
        }
        else
        {
            PRISMATA_ASSERT(false, "Couldn't open file: %s", filename.c_str());
        }
        return ss.str();
    }
}

}