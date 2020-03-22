#pragma once

#include "Common.h"
#include "PartialPlayer.h"

#include "Heuristics.h"

namespace Prismata
{

class PartialPlayer_ActionAbility_ChillGreedyKnapsack : public PartialPlayer
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);

    HealthType freezeEnemyCard(Card & target, std::vector<Card *> & ourChillCards, GameState & state, Move & move);

    bool canContinueChilling(const GameState & state, const PlayerID player) const;

public:
    PartialPlayer_ActionAbility_ChillGreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &) = &Heuristics::CurrentCardValue);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionAbility_ChillGreedyKnapsack(*this));}
};

// sort units in decreasing value of defense so we chill the biggest things first
class ChillKnapsackCompare 
{
    EvaluationType (*_heuristic)(const Card &, const GameState &);
    const GameState & _state;

public:

    ChillKnapsackCompare(EvaluationType (*heuristic)(const Card &, const GameState &), const GameState & state)
        : _heuristic(heuristic)
        , _state(state)
    {
    }

    bool operator() (Card * c1, Card * c2) const
    {
        if (c1->currentHealth() == c2->currentHealth())
        {
            return _heuristic(*c1, _state) < _heuristic(*c2, _state);
        }

        return c1->currentHealth() > c2->currentHealth();
    }
};
}