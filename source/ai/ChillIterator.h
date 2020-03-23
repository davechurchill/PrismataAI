#pragma once

#include "Common.h"
#include "GameState.h"
#include "Heuristics.h"
#include "ChillScenario.h"

namespace Prismata
{

class ChillIterator 
{
    const ChillScenario &       _chillScenario;
    Histogram                   _remainingChillHistogram;
    Histogram                   _remainingDefenderHistogram;

    //std::vector<Action>         _actionStack;
    //std::vector<Action>         _bestActionStack;
    HealthType                  _bestChillUsedLowerBound;
    HealthType                  _bestUsedChill;
    HealthType                  _bestWastedChill;
    HealthType                  _bestSolutionMaxRemainingDefender;
    size_t                      _bestSolutionCount;
    Histogram                   _bestSolutionRemainingDefenderHistogram;

    size_t                      _nodesSearched;
    size_t                      _maxNodes;
    bool                        _solved;
    bool                        _isReset;
    
    void reset();
    bool histogramsEqual(const Histogram & h1, const Histogram & h2) const;

    HealthType calculateChillUsedLowerBound() const;

    void recurse(HealthType usedChill, HealthType totalChillRemaining, HealthType currentBlockerIndex, HealthType partialDefenderChill);
    void printStack(const std::vector<Action> & actionStack, HealthType usedChill, HealthType wastedChill);

    void printRemainingHistograms() const;

public:
    
    ChillIterator(const ChillScenario & scenario);

    bool isSolved() const;
    void solve(const size_t maxNodes = 0);
    void debugSolve(const size_t maxNodes = 0);
    const size_t & getNodesSearched() const;

    const HealthType getBestUsedChill() const;
    const HealthType getBestWastedChill() const;
    const size_t & getBestSolutionCount() const;
};
}