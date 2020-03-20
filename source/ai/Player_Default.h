#pragma once

#include "Common.h"
#include "Player.h"
#include "PartialPlayer.h"

namespace Prismata
{

class Player_Default : public Player
{
 
public:
    Player_Default (const PlayerID & playerID);
    void getMove(const GameState & state, Move & move);

    PlayerPtr clone() { return PlayerPtr(new Player_Default(*this)); }
};
}