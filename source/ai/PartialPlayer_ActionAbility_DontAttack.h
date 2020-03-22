#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_DontAttack : public PartialPlayer
{
 
public:
    PartialPlayer_ActionAbility_DontAttack (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_DontAttack(*this));}
};
}