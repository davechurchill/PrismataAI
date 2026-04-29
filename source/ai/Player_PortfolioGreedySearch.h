#pragma once

#include "Common.h"
#include "MoveIterator_PPPortfolio.h"
#include "Player.h"
#include "Timer.h"

namespace Prismata
{

class Player_PortfolioGreedySearch : public Player
{
    typedef std::vector<size_t> PortfolioSequence;

    MoveIteratorPtr _portfolioIterators[2];
    double          _timeLimitMS;
    size_t          _improvementIterations;
    size_t          _responses;
    Timer           _timer;

    MoveIterator_PPPortfolio & portfolio(const PlayerID player) const;
    PortfolioSequence getDefaultSequence(const PlayerID player) const;
    PortfolioSequence getSeedSequence(const GameState & state, const PlayerID player, const PortfolioSequence & enemySequence);
    PortfolioSequence improve(const GameState & state, const PlayerID player, PortfolioSequence selfSequence, const PortfolioSequence & enemySequence, const size_t iterations);
    double evaluate(const GameState & state, const PlayerID player, const PortfolioSequence & selfSequence, const PortfolioSequence & enemySequence);
    bool timeExpired();

public:

    Player_PortfolioGreedySearch(const PlayerID playerID, const double timeLimitMS, const size_t improvementIterations, const size_t responses, const MoveIteratorPtr & p1Portfolio, const MoveIteratorPtr & p2Portfolio);

    void getMove(const GameState & state, Move & move);
    std::string getDescription();
    PlayerPtr clone();
};

}
