#pragma once

#include "Common.h"
#include "Game.h"
#include "GameState.h"
#include "Heuristics.h"
#include "AllPlayers.h"

typedef std::vector< std::vector<size_t> > vvs;

namespace Prismata
{
 
class DefenseHistogramExperiment 
{
    PlayerPtr               _players[2];

    CardType                _droneType;

    size_t                  _samples;
    size_t                  _samplesPositive;

    std::vector<size_t>     _defenseHistogram;
    std::vector<size_t>     _chillHistogram;

    std::vector<size_t>     _defenseNegativeCounts;
    vvs                     _defenseCounts;
    std::vector<size_t>     _chillCounts;

    void parseState(const GameState & state);


public:
    
    DefenseHistogramExperiment(PlayerPtr p1, PlayerPtr p2, const std::vector<size_t> & histogram);

    void run(const size_t & games);

    void printResults();
};
}