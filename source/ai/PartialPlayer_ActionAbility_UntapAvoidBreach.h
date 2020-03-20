#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_UntapAvoidBreach : public PartialPlayer
{
 
    bool    untapCardsAvoidBreach(GameState & state, Move & move, std::vector<CardID> & cardsToUntap);
    CardID  getNextUntapCardID(const GameState & state, std::vector<CardID> & cards);

public:
    PartialPlayer_ActionAbility_UntapAvoidBreach (const PlayerID & playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_UntapAvoidBreach(*this));}
};
}