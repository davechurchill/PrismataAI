#include "Player_Default.h"
#include "AllPlayers.h"

using namespace Prismata;

Player_Default::Player_Default (const PlayerID playerID)
{
    m_playerID = playerID;
}

void Player_Default::getMove(const GameState & state, Move & move)
{
    GameState currentState(state);

    if (currentState.getActivePhase() == Phases::Defense)
    {
        PartialPlayer_Defense_Default defense(state.getActivePlayer());
        defense.getMove(currentState, move);
    }
    
    if (currentState.getActivePhase() == Phases::Action)
    {
        PartialPlayer_ActionAbility_Default ability(state.getActivePlayer());
        ability.getMove(currentState, move);

        PartialPlayer_ActionBuy_Default buy(state.getActivePlayer());
        buy.getMove(currentState, move);
    }

    if (currentState.getActivePhase() == Phases::Breach)
    {
        PartialPlayer_Breach_Default breach(state.getActivePlayer());
        breach.getMove(currentState, move);
    }
}

 //while (currentState.getActivePhase() == Phases::Defense)
    //{
    //    // block with legal defenders
    //    for (CardID c(0); c < currentState.numCards(_playerID); ++c)
    //    {
    //        Action a(_playerID, ActionTypes::ASSIGN_BLOCKER, currentState.getCard(_playerID, c).getID());
    //        if (currentState.isLegal(a))
    //        {
    //            currentState.doAction(a, false);
    //            move.addAction(a);
    //            break;
    //        }
    //    }
    //}
    //
    //if (currentState.getActivePhase() == Phases::Action)
    //{
    //    // activate everything we can
    //    for (CardID c(0); c < currentState.numCards(_playerID); ++c)
    //    {
    //        Action a(_playerID, ActionTypes::USE_ABILITY, currentState.getCard(_playerID, c).getID());
    //        if (currentState.isLegal(a))
    //        {
    //            currentState.doAction(a, false);
    //            move.addAction(a);
    //        }
    //    }

    //    // buy everything we can
    //    for (CardID c(0); c < currentState.numCardsBuyable(); ++c)
    //    {
    //        Action a(_playerID, ActionTypes::BUY, c);
    //        while (currentState.isLegal(a))
    //        {
    //            currentState.doAction(a, false);
    //            move.addAction(a);
    //        }
    //    }
    //}
    //
    //Action endPhase(_playerID, ActionTypes::END_PHASE, 0);

    //PRISMATA_ASSERT(currentState.isLegal(endPhase), "Player_Default: First End Phase not legal for some reason!");
    //move.addAction(endPhase);
    //currentState.doAction(endPhase, false);

    //PRISMATA_ASSERT(currentState.isLegal(endPhase), "Player_Default: Second End Phase not legal for some reason!");
    //move.addAction(endPhase);
    //currentState.doAction(endPhase, false);

    //while (currentState.getActivePhase() == Phases::Breach && !currentState.isGameOver())
    //{
    //    // block with legal defenders
    //    for (CardID c(0); c < currentState.numCards(currentState.getEnemy(_playerID)); ++c)
    //    {
    //        Action a(_playerID, ActionTypes::ASSIGN_BREACH, currentState.getCard(currentState.getEnemy(_playerID), c).getID());
    //        if (currentState.isLegal(a))
    //        {
    //            currentState.doAction(a, false);
    //            move.addAction(a);
    //            break;
    //        }
    //    }
    //}