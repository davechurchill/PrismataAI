#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "BlockIterator.h"

namespace Prismata
{

class PartialPlayer_Defense_Solver : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState & state, const HealthType &);

public:

    PartialPlayer_Defense_Solver (const PlayerID & playerID, EvaluationType (*heuristic)(const Card &, const GameState & state, const HealthType &) = &Heuristics::DamageLoss_WillCost);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_Defense_Solver(*this));}
};
}