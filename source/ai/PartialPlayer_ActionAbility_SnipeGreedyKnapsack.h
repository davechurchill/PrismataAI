#pragma once

#include "Common.h"
#include "PartialPlayer.h"

#include "Heuristics.h"
#include <map>

namespace Prismata
{

class PartialPlayer_ActionAbility_SnipeGreedyKnapsack : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);

public:
    PartialPlayer_ActionAbility_SnipeGreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &) = &Heuristics::CurrentCardValue);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_SnipeGreedyKnapsack(*this));}
};

// sort units in decreasing value so that we kill higher valued units first
class SnipeKnapsackCompare 
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);
    const GameState & _state;

public:

    SnipeKnapsackCompare(EvaluationType (*heuristic)(const Card &, const GameState &), const GameState & state)
        : _heuristic(heuristic)
        , _state(state)
    {
    }

    bool operator() (const CardID c1, const CardID c2) const
    {
        return _heuristic(_state.getCardByID(c1), _state) > _heuristic(_state.getCardByID(c2), _state);
    }
};
}