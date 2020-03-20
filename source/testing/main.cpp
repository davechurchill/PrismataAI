#include "Prismata.h"
#include "PrismataAI.h"
#include "Benchmarks.h"
#include <iostream>

using namespace Prismata;

int main(int argc, char* argv[])
{
    printf("Benchmarks!\n");

    srand((unsigned int)time(NULL));

    // read all the configuration settings
    std::string configDir = "asset/config/";

    printf("Initializing card library\n");
    Prismata::InitFromCardLibrary(configDir + "cardLibrary.jso");

    printf("Parsing AI Parameters\n");
    Prismata::AIParameters::Instance().parseFile(configDir + "config.txt");
    
    printf("Running Benchmarks\n");
    Benchmarks::DoBenchmarks(configDir + "config.txt");
     
    return 0;
}