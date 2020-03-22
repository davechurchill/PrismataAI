#include "PartialPlayer_ActionAbility_ActivateAll.h"

using namespace Prismata;

PartialPlayer_ActionAbility_ActivateAll::PartialPlayer_ActionAbility_ActivateAll(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_ActivateAll::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // activate everything we can
    while (true)
    {
        bool cardActivated = false;

        for (const auto & cardID : state.getCardIDs(_playerID))
        {
            if (!state.getCardByID(cardID).getType().hasAbility())
            {
                continue;
            }

            Action a(_playerID, ActionTypes::USE_ABILITY, cardID);

            if (state.isLegal(a))
            {
                state.doAction(a);
                move.addAction(a);
                cardActivated = true;
                
                // we have to break here in case of iterator invalidation
                // activating a card could cause another one to die
                break;
            }
        }

        if (!cardActivated)
        {
            break;
        }
    }
}
