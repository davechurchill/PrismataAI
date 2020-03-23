#pragma once

#include "Common.h"
#include "GameState.h"
#include "Heuristics.h"

typedef std::vector<size_t> Histogram;

namespace Prismata
{
 
class ChillScenario 
{
    size_t                      _numDefenders;
    HealthType                  _totalDefense;
    HealthType                  _largestDefender;
    HealthType                  _smallestDefender;

    size_t                      _numChillers;
    HealthType                  _totalChill;
    HealthType                  _largestChiller;
    HealthType                  _smallestChiller;
    HealthType                  _totalDefenseBought;

    Histogram                   _chillHistogram;
    Histogram                   _defenseHistogram;

    size_t                      _modifications;
    
    HealthType                  calculateTopDownChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const;
    HealthType                  calculateTotalsChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const;
    HealthType                  calculateTwosChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const;
    HealthType                  calculateThreesChillWastedHeuristic(std::vector<size_t> & chillHistogram, std::vector<size_t> & defenseHistogram) const;

public:
    
    ChillScenario();
    ChillScenario(const GameState & state, const PlayerID chillPlayer);

    const size_t & getNumChillers() const;
    const size_t & getNumDefenders() const;
    const size_t & getChillers(const size_t & chill) const;
    const size_t & getDefenders(const size_t & chill) const;
    const size_t & getModifications() const;

    const HealthType getTotalDefense() const;
    const HealthType getTotalChill() const;
    const HealthType getTotalDefenseBought() const;
    const HealthType getLargestDefender() const;
    const HealthType getLargestChiller() const;

    const Histogram & getDefenseHistogram() const;
    const Histogram & getChillHistogram() const;

    HealthType calculateUsedChill(const size_t & maxSolverIterations);
    HealthType calculateHeuristicUsedChill();
    HealthType calculateExactUsedChill() const;

    void addChiller(const HealthType chill, const size_t n = 1);
    void addChiller(const CardType type, const size_t n = 1);
    void addChiller(const Card & card, const size_t n = 1);
    void removeChiller(const HealthType chill, const size_t n = 1);
    void removeChiller(const CardType type, const size_t n = 1);
    void removeChiller(const Card & card, const size_t n = 1);

    void buyDefender(const CardType type);
    void sellDefender(const CardType type);
    void addDefender(const HealthType health, const size_t n = 1);
    void addDefender(const CardType type, const size_t n = 1);
    void addDefender(const Card & card, const size_t n = 1);
    void removeDefender(const HealthType health, const size_t n = 1);
    void removeDefender(const CardType type, const size_t n = 1);
    void removeDefender(const Card & card, const size_t n = 1);

    void setRandomData(const size_t & histogramMinIndex, const size_t & histogramMaxIndex, const size_t & maxHistogramValue);
    void print() const;

    GameState constructGameState() const;
};
}