#include "PartialPlayer_ActionAbility_FrontlineGreedyKnapsack.h"

using namespace Prismata;

PartialPlayer_ActionAbility_FrontlineGreedyKnapsack::PartialPlayer_ActionAbility_FrontlineGreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &))
    : _heuristic(heuristic)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_FrontlineGreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    // make a vector of all enemy frontline cards
    std::vector<Card *> enemyFrontlineCards;
    for (const auto & cardID : state.getCardIDs(state.getEnemy(_playerID)))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.getType().isFrontline())
        {
            enemyFrontlineCards.push_back(&(Card &)card);
        }
    }

    // if we got nothing to do don't do nothing
    if (enemyFrontlineCards.empty())
    {
        return;
    }
    
    // sort the cards in the given frontline priority
    std::sort(enemyFrontlineCards.begin(), enemyFrontlineCards.end(), FrontlineKnapsackCompare(_heuristic, state));

    // frontline cards in order of priority
    Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    
    // keep frontlining until we can't find any more legal targets
    while (state.getActivePhase() == Phases::Action)
    {
        bool foundFrontline = false;
        for (size_t c(0); c < enemyFrontlineCards.size(); ++c)
        {
            if (!enemyFrontlineCards[c])
            {
                continue;
            }

            const Card & card(*enemyFrontlineCards[c]);
            Action a(_playerID, ActionTypes::ASSIGN_FRONTLINE, card.getID());

            if (state.isLegal(a))
            {
                foundFrontline = true;
                state.doAction(a);
                move.addAction(a);
                enemyFrontlineCards[c] = NULL;
                break;
            }
        }

        if (!foundFrontline)
        {
            break;
        }
    }
}
