#pragma once

#include "Common.h"
#include "Player.h"

namespace Prismata
{

class Player_Random : public Player
{
public:
    Player_Random (const PlayerID & playerID);
    void getMove(const GameState & state, Move & move);

    PlayerPtr clone() { return PlayerPtr(new Player_Random(*this)); }
};
}