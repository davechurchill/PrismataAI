#include "PartialPlayer_Defense_Random.h"

using namespace Prismata;

PartialPlayer_Defense_Random::PartialPlayer_Defense_Random(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::DEFENSE;
}

void PartialPlayer_Defense_Random::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Defense)
    {
        return;
    }

    std::vector<Action> legalActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Defense)
    {
        legalActions.clear();
        state.generateLegalActions(legalActions);

        Action a = legalActions[rand() % legalActions.size()];
        state.doAction(a);
        move.addAction(a);
    }
}
