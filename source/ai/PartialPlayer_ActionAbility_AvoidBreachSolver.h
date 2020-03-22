#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "Heuristics.h"
#include "AvoidBreachBuyIterator.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_AvoidBreachSolver : public PartialPlayer
{
    BreachIteratorParameters _params;

public:

    PartialPlayer_ActionAbility_AvoidBreachSolver (const PlayerID playerID, const BreachIteratorParameters & params);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_AvoidBreachSolver(*this));}
};

}