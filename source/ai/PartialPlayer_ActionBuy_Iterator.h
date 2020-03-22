#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionBuy_Iterator : public PartialPlayer
{
    CardID numBuyable(const GameState & state, const Resources & resource, const CardType & type);

public:

    PartialPlayer_ActionBuy_Iterator (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_Iterator(*this));}
};
}