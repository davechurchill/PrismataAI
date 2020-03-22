#include "Player_PPSequence.h"
#include "AllPlayers.h"
#include <memory>

using namespace Prismata;

Player_PPSequence::Player_PPSequence(const PlayerID playerID, const PPSequence & sequence)
{
    m_playerID = playerID;
    _sequence = sequence;
}

void Player_PPSequence::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(_sequence.size() == PPPhases::NUM_PHASES, "Input PPSequence isn't of the correct size");

    GameState currentState(state);
    Action endPhase = Action(m_playerID, ActionTypes::END_PHASE, 0);
        
    // get the DEFENSE phase moves. defense partial player will end defense phase by itself
    if (currentState.getActivePlayer() == m_playerID && currentState.getActivePhase() == Phases::Defense)
    {
        _sequence[PPPhases::DEFENSE]->getMove(currentState, move);
    }

    // get the action ability moves, these need to happen before buying in case they generate money
    if (currentState.getActivePlayer() == m_playerID && currentState.getActivePhase() == Phases::Action)
    {
        _sequence[PPPhases::ACTION_ABILITY]->getMove(currentState, move);
    }

    // get the buy moves, these partial players will NOT end the phase so they can be interwoven
    if (currentState.getActivePlayer() == m_playerID && currentState.getActivePhase() == Phases::Action)
    {
        _sequence[PPPhases::ACTION_BUY]->getMove(currentState, move);
        
        PRISMATA_ASSERT(currentState.isLegal(endPhase), "We should be able to end phase after action phase");

        // have to end after the buy phase
        move.addAction(endPhase);
        currentState.doAction(endPhase);
    }

    // get the breach actions, breach partial player will end phase by itself
    if (currentState.getActivePlayer() == m_playerID && currentState.getActivePhase() == Phases::Breach)
    {
        _sequence[PPPhases::BREACH]->getMove(currentState, move);
    }

    // make sure we are in the confirm phase, and end the turn
    PRISMATA_ASSERT(currentState.getActivePhase() == Phases::Confirm, "Partial player sequence did not end in CONFIRM phase");
    
    move.addAction(endPhase);
}

PlayerPtr Player_PPSequence::clone() 
{ 
    return PlayerPtr(new Player_PPSequence(m_playerID, _sequence.clone()));
}

std::string Player_PPSequence::getDescription() 
{ 
    return m_description + "\n" + _sequence.getDescription(); 
};