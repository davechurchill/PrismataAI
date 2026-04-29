#pragma once

#include "AlphaBetaSearchParameters.hpp"
#include "MoveIterator.h"
#include "Player.h"
#include "Timer.h"

namespace Prismata
{

class Player_RootParallelAlphaBeta : public Player
{
    struct RootResult
    {
        Move move;
        double value = -std::numeric_limits<double>::max();
    };

    AlphaBetaSearchParameters _params;

    bool moveIsLegal(const GameState & state, const Move & move) const;
    double evaluate(const GameState & state) const;
    double alphaBeta(const GameState & state,
                     const size_t depth,
                     const size_t maxDepth,
                     double alpha,
                     double beta,
                     Timer & timer,
                     bool & timedOut) const;
    RootResult searchRootChild(const GameState & child, const Move & move) const;

public:
    Player_RootParallelAlphaBeta(const PlayerID playerID, const AlphaBetaSearchParameters & params);

    void getMove(const GameState & state, Move & move);
    std::string getDescription();
    PlayerPtr clone();
};

}
