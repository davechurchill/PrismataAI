#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "CardFilter.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_AttackDefault : public PartialPlayer
{
    std::vector<CardID> _attackers;
    CardFilter          _filter;

    void getAttackers(const GameState & state, std::vector<CardID> & attackers);
 
public:

    PartialPlayer_ActionAbility_AttackDefault (const PlayerID & playerID, const CardFilter & filter);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_AttackDefault(*this));}
};
}