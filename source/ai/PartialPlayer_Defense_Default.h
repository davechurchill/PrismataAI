#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_Defense_Default : public PartialPlayer
{
 
public:
    PartialPlayer_Defense_Default (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_Defense_Default(*this));}
};
}