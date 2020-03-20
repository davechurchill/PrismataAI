#include "PartialPlayer_Breach_Random.h"

using namespace Prismata;

PartialPlayer_Breach_Random::PartialPlayer_Breach_Random(const PlayerID & playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::BREACH;
}

void PartialPlayer_Breach_Random::getMove(GameState & state, Move & move)
{ 
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Breach)
    {
        return;
    }

    Action endPhase(_playerID, ActionTypes::END_PHASE, 0);

    std::vector<Action> legalActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Breach)
    {
        legalActions.clear();
        state.generateLegalActions(legalActions);
        
        Action a = legalActions[rand() % legalActions.size()];
        state.doAction(a);
        move.addAction(a);

        // if the move we generated was to undo chill, we must insert an end phase in order to breakthrough
        if (a.getType() == ActionTypes::UNDO_CHILL)
        {
            PRISMATA_ASSERT(state.isLegal(endPhase), "We should be able to breakthrough after undo chill");
            state.doAction(endPhase);
            move.addAction(endPhase);
        }
    }
}
