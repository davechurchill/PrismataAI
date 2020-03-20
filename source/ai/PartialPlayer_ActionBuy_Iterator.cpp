#include "PartialPlayer_ActionBuy_Iterator.h"

using namespace Prismata;

PartialPlayer_ActionBuy_Iterator::PartialPlayer_ActionBuy_Iterator(const PlayerID & playerID)
{
 _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

void PartialPlayer_ActionBuy_Iterator::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // buy everything we can
    for (CardID c(0); c < state.numCardsBuyable(); ++c)
    {
        Action a(_playerID, ActionTypes::BUY, c);
        while (state.isLegal(a))
        {
            state.doAction(a);
            move.addAction(a);
        }
    }
    
}

CardID PartialPlayer_ActionBuy_Iterator::numBuyable(const GameState & state, const Resources & resourceRemaining, const CardType & type)
{
    while (true)
    {
        
    }
}