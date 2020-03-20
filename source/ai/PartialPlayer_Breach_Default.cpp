#include "PartialPlayer_Breach_Default.h"

using namespace Prismata;

PartialPlayer_Breach_Default::PartialPlayer_Breach_Default(const PlayerID & playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::BREACH;
}

void PartialPlayer_Breach_Default::getMove(GameState & state, Move & move)
{ 
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Breach)
    {
        return;
    }

    int breachLoops = 0;
    Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    while (state.getActivePhase() == Phases::Breach && !state.isGameOver() && !state.isLegal(end))
    {
        breachLoops++;
        bool found = false;
        for (const auto & cardID : state.getCardIDs(state.getEnemy(_playerID)))
        {
            Action a(_playerID, ActionTypes::ASSIGN_BREACH, cardID);
            if (state.isLegal(a))
            {
                found = true;
                state.doAction(a);
                move.addAction(a);
                break;
            }
        }

        PRISMATA_ASSERT(found, "Should have found a breachable");
    }

    PRISMATA_ASSERT(state.isLegal(end), "Should be able to end breach phase");

    state.doAction(end);
    move.addAction(end);
}
