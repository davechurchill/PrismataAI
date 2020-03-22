#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "BuyLimits.h"
#include "CardFilter.h"

namespace Prismata
{

enum TechHeuristics
{
    ELYOT_FORMULA           = 0,
    ELYOT_FORMULA_BALANCED  = 1,
    DIVERSIFY               = 2,
    ELYOT_FORMULA_PLAYOUT   = 3
};

class PartialPlayer_ActionBuy_TechHeuristic : public PartialPlayer
{
    size_t _heuristicType;
    bool greaterThan(double * v1, double * v2, size_t size);
    void getMovesElyotFormula(GameState & state, Move & move, bool balanced);
    void getMovesDiversify(GameState & state, Move & move);

public:

    PartialPlayer_ActionBuy_TechHeuristic (const PlayerID playerID, const size_t & heuristicType);
    
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_TechHeuristic(*this));}
};
}