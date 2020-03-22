#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_EconomyDefault : public PartialPlayer
{
 
public:
    PartialPlayer_ActionAbility_EconomyDefault (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_EconomyDefault(*this));}
};
}