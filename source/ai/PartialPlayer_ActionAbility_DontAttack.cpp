#include "PartialPlayer_ActionAbility_DontAttack.h"

using namespace Prismata;

PartialPlayer_ActionAbility_DontAttack::PartialPlayer_ActionAbility_DontAttack(const PlayerID & playerID)
{
 _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_DontAttack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

     // activate everything we can except don't attack
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        Action a(_playerID, ActionTypes::USE_ABILITY, cardID);
        if (state.isLegal(a) && (state.getCardByID(cardID).getType().getAttack() == 0))
        {
            state.doAction(a);
            move.addAction(a);
        }
    }
}
