#pragma once

#include "Common.h"

#include "Player.h"

namespace Prismata
{

class Player_GUI : public Player
{

public:

    Player_GUI (const PlayerID playerID);
    void getMove(const GameState & state, Move & move);

    PlayerPtr clone() { return PlayerPtr(new Player_GUI(*this)); }
};
}
