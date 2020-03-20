#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_AvoidEconomyWaste : public PartialPlayer
{
    std::vector<CardID> _toActivate;
    std::vector<CardID> _toUndo;
    bool shouldUseCardAbility(const GameState & state, const CardID & cardID) const;
    bool shouldUndoCardAbility(const GameState & state, const CardID & cardID) const;

public:

    PartialPlayer_ActionAbility_AvoidEconomyWaste(const PlayerID & playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_AvoidEconomyWaste(*this));}
};
}