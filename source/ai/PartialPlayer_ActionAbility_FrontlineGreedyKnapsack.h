#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_FrontlineGreedyKnapsack : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);

public:
    PartialPlayer_ActionAbility_FrontlineGreedyKnapsack(const PlayerID & playerID, EvaluationType (*heuristic)(const Card &, const GameState &) = &Heuristics::CurrentCardValue);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_FrontlineGreedyKnapsack(*this));}
};

// sort units in decreasing value so that we kill higher valued units first
class FrontlineKnapsackCompare 
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);
    const GameState & _state;

public:

    FrontlineKnapsackCompare(EvaluationType (*heuristic)(const Card &, const GameState &), const GameState & state)
        : _heuristic(heuristic)
        , _state(state)
    {
    }

    bool operator() (Card * c1, Card * c2) const
    {
        return _heuristic(*c1, _state) > _heuristic(*c2, _state);
    }
};
}