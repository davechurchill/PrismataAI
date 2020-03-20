#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"

namespace Prismata
{

class Player_RandomFromIterator : public Player
{
    MoveIteratorPtr _moveIterator;

public:

    Player_RandomFromIterator (const PlayerID & playerID, const MoveIteratorPtr & moveIterator);
    void getMove(const GameState & state, Move & move);
    std::string getDescription();

    PlayerPtr clone();
};
}