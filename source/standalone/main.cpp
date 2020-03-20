#include "Prismata.h"
#include "PrismataAI.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>

#ifdef WIN32
    #include <Windows.h>
#endif

using namespace Prismata;

void printVersionInfo()
{
    std::stringstream ss;
    ss << "C++ AI compiled on: " << __DATE__ << " at " << __TIME__ << std::endl;
 
    printf("%s", ss.str().c_str());
}

int main(int argc, char *argv[])
{
#ifdef WIN32
    // disables pop up box on crash
    DWORD dwMode = SetErrorMode(SEM_NOGPFAULTERRORBOX);
    SetErrorMode(dwMode | SEM_NOGPFAULTERRORBOX);
#endif
    
    srand((unsigned int)time(NULL));
    
    // get the input line from stdin
    std::string inputLine;
    std::getline(std::cin, inputLine);

    // if the input string is 'version' print to stdout
    if (inputLine == "version")
    {
        printVersionInfo();
        return 0;
    }

    bool debugPrintToFile = false;

    if (debugPrintToFile)
    {
        // test print the line we got to a file for debugging
        std::ofstream fout("stdin_contents.txt", std::ofstream::out);
        fout << inputLine;
        fout.close();
    }

    // initialize, compute the move, and print the resulting move
    std::string moveString = AITools::InitializeAIAndGetAIMove(inputLine);

    // remove newlines from the resulting string so it counts as one line for stdout
    for (size_t i(0); i < moveString.size(); ++i)
    {
        #ifdef WIN32
        if (moveString[i] == '\n' || moveString[i] == '\r\n')
        #else
        if (moveString[i] == '\n')
        #endif
        {
            moveString[i] = ' ';
        }
    }

    if (debugPrintToFile)
    {
        std::ofstream fout("stdout_move_string.txt", std::ofstream::out);
        fout << moveString;
        fout.close();
    }
    
    std::cout << moveString << "\n";

    return 0;
}
