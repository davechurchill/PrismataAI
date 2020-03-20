#include "PartialPlayer_ActionAbility_Random.h"

using namespace Prismata;

PartialPlayer_ActionAbility_Random::PartialPlayer_ActionAbility_Random(const PlayerID & playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_Random::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::vector<Action> legalActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == _phaseID)
    {
        legalActions.clear();
        state.generateLegalActions(legalActions);

        Action a = legalActions[rand() % legalActions.size()];
        state.doAction(a);
        move.addAction(a);
    }
}
