#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "CardFilter.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_ActivateUtility : public PartialPlayer
{
    std::vector<CardID> _utility;
    CardFilter          _filter;
    void getUtility(const GameState & state, std::vector<CardID> & utility);
    bool isUtilityCard(const Card & card) const;

public:

    PartialPlayer_ActionAbility_ActivateUtility (const PlayerID & playerID, const CardFilter & filter);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_ActivateUtility(*this));}
};

}