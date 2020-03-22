#include "PartialPlayer_Defense_Default.h"

using namespace Prismata;

PartialPlayer_Defense_Default::PartialPlayer_Defense_Default(const PlayerID playerID)
{
 _playerID = playerID;
    _phaseID = PPPhases::DEFENSE;
}

void PartialPlayer_Defense_Default::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Defense)
    {
        return;
    }

    Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    while (state.getActivePhase() == Phases::Defense && !state.isLegal(end))
    {
        // block with legal defenders
        for (const auto & cardID : state.getCardIDs(_playerID))
        {
            Action a(_playerID, ActionTypes::ASSIGN_BLOCKER, cardID);
            if (state.isLegal(a))
            {
                state.doAction(a);
                move.addAction(a);
                break;
            }
        }
    }

    PRISMATA_ASSERT(state.isLegal(end), "Should be able to end defense phase");

    state.doAction(end);
    move.addAction(end);
}
