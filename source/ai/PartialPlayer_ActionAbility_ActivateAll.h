#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_ActivateAll : public PartialPlayer
{
 
public:
    PartialPlayer_ActionAbility_ActivateAll (const PlayerID & playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_ActivateAll(*this));}
};
}