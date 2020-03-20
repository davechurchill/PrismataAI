#pragma once

#include "Common.h"
#include "PartialPlayer.h"

#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_ChillSolver : public PartialPlayer
{
    size_t _maxIterations;

public:

    PartialPlayer_ActionAbility_ChillSolver(const PlayerID & playerID, const size_t maxIterations = 0);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_ChillSolver(*this));}
};

}