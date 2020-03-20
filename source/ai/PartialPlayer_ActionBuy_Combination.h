#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionBuy_Combination : public PartialPlayer
{
    std::vector<PPPtr>  _combo;
    std::vector<size_t> _movesGenerated;

public:
    PartialPlayer_ActionBuy_Combination (const PlayerID & playerID, const std::vector<PPPtr> & combo);
    void getMove(GameState & state, Move & move);
    virtual std::string getDescription(size_t depth = 0);
    virtual void setBuyLimits(const BuyLimits & buyLimits);

    PPPtr clone();
};
}