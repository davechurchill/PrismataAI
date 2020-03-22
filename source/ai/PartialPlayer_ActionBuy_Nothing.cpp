#include "PartialPlayer_ActionBuy_Nothing.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Nothing::PartialPlayer_ActionBuy_Nothing(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Nothing::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // do nothing!
}
