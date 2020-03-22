#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionBuy_Nothing : public PartialPlayer
{

public:
    PartialPlayer_ActionBuy_Nothing (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_Nothing(*this));}
};
}