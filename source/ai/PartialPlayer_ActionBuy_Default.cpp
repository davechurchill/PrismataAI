#include "PartialPlayer_ActionBuy_Default.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Default::PartialPlayer_ActionBuy_Default(const PlayerID & playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Default::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // buy everything we can
    for (CardID c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardBuyable & cardBuyable(state.getCardBuyableByIndex(c));

        if (CardTypes::IsBaseSet(cardBuyable.getType()))
        {
            Action a(_playerID, ActionTypes::BUY, cardBuyable.getType().getID());
            while (state.isLegal(a))
            {
                state.doAction(a);
                move.addAction(a);
            }
        }
    }
}
