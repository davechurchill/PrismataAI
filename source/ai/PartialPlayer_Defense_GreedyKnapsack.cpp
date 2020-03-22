#include "PartialPlayer_Defense_GreedyKnapsack.h"

using namespace Prismata;

PartialPlayer_Defense_GreedyKnapsack::PartialPlayer_Defense_GreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &))
    : _heuristic(heuristic)
{
    _playerID = playerID;
    _phaseID = PPPhases::DEFENSE;
}

void PartialPlayer_Defense_GreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Defense)
    {
        return;
    }

    // make a vector of pointers to all legal blockers
    std::vector<Card *> blockers;
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        Action a(_playerID, ActionTypes::ASSIGN_BLOCKER, cardID);
        if (state.isLegal(a))
        {
            blockers.push_back(&(Card &)card);
        }
    }

    // sort the blockers in the given priority
    std::sort(blockers.begin(), blockers.end(), DefenseKnapsackCompare(_heuristic, state));

    // block with the blockers in the given priority
    Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    for (size_t c(0); c<blockers.size() && (state.getActivePhase() == Phases::Defense) && !state.isLegal(end); ++c)
    {
        Action a(_playerID, ActionTypes::ASSIGN_BLOCKER, blockers[c]->getID());
        PRISMATA_ASSERT(state.isLegal(a), "We should still be able to block with this card");

        state.doAction(a);
        move.addAction(a);
    }

    PRISMATA_ASSERT(state.isLegal(end), "Should be able to end defense phase");

    state.doAction(end);
    move.addAction(end);
}
