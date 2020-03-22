#pragma once

#include "Common.h"
#include "PartialPlayer.h"

#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_Breach_GreedyKnapsack : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState &, const HealthType &);
    double  _totalBreachDamageLoss;
    bool    _lowTechPriority;

public:

    PartialPlayer_Breach_GreedyKnapsack(const PlayerID playerID, bool lowTechPriority, EvaluationType (*heuristic)(const Card &, const GameState &, const HealthType &) = &Heuristics::DamageLoss_WillCost);
    void getMove(GameState & state, Move & move);
    double getTotalBreachDamageLoss() const;

    PPPtr clone() { return PPPtr(new PartialPlayer_Breach_GreedyKnapsack(*this));}
};

// sort units in decreasing value so that we kill higher valued units first
class BreachKnapsackCompare 
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);
    const GameState & _state;

public:

    BreachKnapsackCompare(EvaluationType (*heuristic)(const Card &, const GameState &), const GameState & state)
        : _heuristic(heuristic)
        , _state(state)
    {
    }

    bool operator() (CardID c1, CardID c2) const
    {
        return _heuristic(_state.getCardByID(c1), _state) > _heuristic(_state.getCardByID(c2), _state);
    }
};
}