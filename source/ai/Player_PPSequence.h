#pragma once

#include "Common.h"
#include "Player.h"
#include "PartialPlayer.h"
#include "PartialPlayerSequence.h"

namespace Prismata
{

class Player_PPSequence : public Player
{
    PPSequence _sequence;
public:
    Player_PPSequence (const PlayerID & playerID, const PPSequence & sequence);
    void getMove(const GameState & state, Move & move);

    std::string getDescription();

    PlayerPtr clone();
};
}