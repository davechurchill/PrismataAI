#include "PartialPlayer_ActionBuy_Random.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Random::PartialPlayer_ActionBuy_Random(const PlayerID playerID)
{
 _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Random::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::vector<Action> legalActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Action)
    {
        legalActions.clear();
        state.generateLegalActions(legalActions);

        Action a = legalActions[rand() % legalActions.size()];

        // buy players should never actually end the phase, so if we chose end phase just exit
        if (a.getType() == ActionTypes::END_PHASE)
        {
            return;
        }
        else
        {
            state.doAction(a);
            move.addAction(a);
        }
    }
}
