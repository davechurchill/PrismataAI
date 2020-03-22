#pragma once

#include "Common.h"
#include "PartialPlayer.h"

#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_Defense_GreedyKnapsack : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);

public:
    PartialPlayer_Defense_GreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &) = &Heuristics::CurrentCardValue);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_Defense_GreedyKnapsack(*this));}
};

// sort units in increasing value so that we will block with lower valued units first
class DefenseKnapsackCompare 
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);
    const GameState & _state;

public:

    DefenseKnapsackCompare(EvaluationType (*heuristic)(const Card &, const GameState &), const GameState & state)
        : _heuristic(heuristic)
        , _state(state)
    {
    }

    bool operator() (Card * c1, Card * c2) const
    {
        return _heuristic(*c1, _state) < _heuristic(*c2, _state);
    }
};
}