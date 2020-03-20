#include "PrismataAI.h"
#include "Benchmarks.h"
#include "PlayerBenchmark.h"
#include "Tournament.h"
#include <thread>

using namespace Prismata;

void Benchmarks::DoBenchmarks(const std::string & filename)
{
    rapidjson::Document document;
    bool parsingFailed = document.Parse(FileUtils::ReadFile(filename).c_str()).HasParseError();

    PRISMATA_ASSERT(!parsingFailed, "Couldn't parse benchmarks file");

    PRISMATA_ASSERT(document.HasMember("Benchmarks"), "JSON has no Benchmarks member");
    PRISMATA_ASSERT(document["Benchmarks"].IsArray(), "JSON Benchmarks member is not an array");

    std::vector<std::thread> threads;

    const rapidjson::Value & benchmarks = document["Benchmarks"];
    for (size_t b(0); b < benchmarks.Size(); ++b)
    {
        bool run = benchmarks[b].HasMember("run") && benchmarks[b]["run"].IsBool() && benchmarks[b]["run"].GetBool();
       
        PRISMATA_ASSERT(benchmarks[b].HasMember("name") && benchmarks[b]["name"].IsString(), "Benchmark must have name string member");
        const std::string & name = benchmarks[b]["name"].GetString();
        
        // must have a true 'run' bool to run the benchmark
        if (!run)
        {
            continue;
        }

        PRISMATA_ASSERT(benchmarks[b].HasMember("type") && benchmarks[b]["type"].IsString(), "Benchmark must have type string member");

        const std::string & benchmarkType = benchmarks[b]["type"].GetString();

        if (benchmarkType == "PlayerBenchmark")
        {
            PlayerBenchmark bm(benchmarks[b]);
            threads.emplace_back(DoPlayerBenchmark, bm);
        }
        else if (benchmarkType == "ChillIterator")
        {
            DoChillIteratorBenchmarkJSON(benchmarks[b]);
        }
        else if (benchmarkType == "Tournament")
        {
            DoTournamentBenchmark(benchmarks[b]);
        }
        else
        {
            PRISMATA_ASSERT(false, "Unknown Benchmark type: %s", benchmarkType.c_str());
        }
    }

    for (auto & t : threads)
    {
        t.join();    
    }
}

void Benchmarks::DoTournamentBenchmark(const rapidjson::Value & value)
{
    Tournament tournament(value);
    tournament.run();
}

void Benchmarks::DoPlayerBenchmark(const PlayerBenchmark & benchmark)
{
    PlayerBenchmark b(benchmark);
    b.run();
}

void Benchmarks::DoChillIteratorBenchmarkJSON(const rapidjson::Value & value)
{
    PRISMATA_ASSERT(value.HasMember("TimeLimitMS") && value["TimeLimitMS"].IsInt(), "ChillIteratorBenchmark must have TimeLimitMS int");
    PRISMATA_ASSERT(value.HasMember("HistogramMinIndex") && value["HistogramMinIndex"].IsInt(), "ChillIteratorBenchmark must have HistogramMinIndex int");
    PRISMATA_ASSERT(value.HasMember("HistogramMaxIndex") && value["HistogramMaxIndex"].IsInt(), "ChillIteratorBenchmark must have HistogramMaxIndex int");
    PRISMATA_ASSERT(value.HasMember("HistogramMaxValue") && value["HistogramMaxValue"].IsInt(), "ChillIteratorBenchmark must have HistogramMaxValue int");

    DoChillIteratorBenchmark(value["TimeLimitMS"].GetInt(), value["HistogramMinIndex"].GetInt(), value["HistogramMaxIndex"].GetInt(), value["HistogramMaxValue"].GetInt());
}

void Benchmarks::DoChillIteratorBenchmark(size_t timeLimitMS, size_t histogramMinIndex, size_t histogramMaxIndex, size_t histogramMaxValue)
{
    std::cout << "\nStarting " << timeLimitMS << "ms ChillIterator Benchmark: \n";

    Timer t;
    t.start();

    double msElapsed = 0;
    size_t totalChillScenarios = 0;
    size_t totalSolveIterations = 0;

    while (msElapsed < timeLimitMS)
    {
        ChillScenario chillScenario;
        chillScenario.setRandomData(histogramMinIndex, histogramMaxIndex, histogramMaxValue);

        ChillIterator chillIterator(chillScenario);
        chillIterator.solve();
        
        totalSolveIterations += chillIterator.getNodesSearched();

        totalChillScenarios++;

        msElapsed = t.getElapsedTimeInMilliSec();
    }

    double ms = msElapsed;
    double gps = (totalChillScenarios / ms) * 1000;

    std::cout << "  Solved " << totalChillScenarios << "  in " << ms << " ms @ " << gps << " games per second" << std::endl;
    std::cout << "  Solves took an average of " << (totalSolveIterations / totalChillScenarios) << " iterations @ " << (totalSolveIterations / ms) * 1000 << " iterations per second\n\n";
}
