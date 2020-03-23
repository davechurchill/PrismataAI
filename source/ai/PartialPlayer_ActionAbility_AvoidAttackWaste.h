#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_AvoidAttackWaste : public PartialPlayer
{
    void        getUntappableAttackers(const GameState & state, std::vector<CardID> & untappable);
    void        getAbilityAttackCostCards(const GameState & state, std::vector<CardID> & cards);
    HealthType  calculateLossDecreaseThreshold(const GameState & state, const double & originalLoss);
    void        untapLifeSpanOneHeuristic(GameState & state, Move & move, std::vector<CardID> & attackingCards);
    void        untapAttackingCards(GameState & state, Move & move, std::vector<CardID> & attackingCards, const HealthType lossDecreaseAttackThreshold);
    void        untapAbilityAttackCostCards(GameState & state, Move & move, std::vector<CardID> & attackingCards, const HealthType lossDecreaseAttackThreshold);

    bool        weWillWinOnThisAttack(const GameState & state);

public:
    PartialPlayer_ActionAbility_AvoidAttackWaste (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_AvoidAttackWaste(*this));}
};

class WasteCompare 
{
    const GameState & _state;

public:

    WasteCompare(const GameState & state);
    bool operator() (CardID c1, CardID c2) const;
};
}