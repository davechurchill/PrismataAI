#include "PartialPlayer_ActionBuy_Random.h"
#include "Random.h"

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
    std::vector<Action> buyActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Action)
    {
        if (move.size() >= MAX_MOVE_ACTIONS)
        {
            return;
        }

        legalActions.clear();
        state.generateLegalActions(legalActions);

        buyActions.clear();
        for (const Action & action : legalActions)
        {
            if (action.getType() == ActionTypes::BUY || action.getType() == ActionTypes::END_PHASE)
            {
                buyActions.push_back(action);
            }
        }

        if (buyActions.empty())
        {
            return;
        }

        Action a = buyActions[Random::Int(buyActions.size())];

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
