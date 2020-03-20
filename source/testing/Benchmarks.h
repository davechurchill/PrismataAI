#pragma once

#include "Prismata.h"
#include "rapidjson/rapidjson.h"
#include "PlayerBenchmark.h"

namespace Prismata
{

namespace Benchmarks
{
    void DoBenchmarks(const std::string & filename);

    void DoTournamentBenchmark(const rapidjson::Value & value);
    void DoPlayerBenchmark(const PlayerBenchmark & benchmark);

    void DoChillIteratorBenchmarkJSON(const rapidjson::Value & value);
    void DoChillIteratorBenchmark(size_t timeLimitMS, size_t histogramMinIndex, size_t histogramMaxIndex, size_t histogramMaxValue);
}
}
