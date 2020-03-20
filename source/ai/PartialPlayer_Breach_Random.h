#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_Breach_Random : public PartialPlayer
{
 
public:
    PartialPlayer_Breach_Random (const PlayerID & playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_Breach_Random(*this));}
};
}