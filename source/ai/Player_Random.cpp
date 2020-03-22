#include "Player_Random.h"

using namespace Prismata;

Player_Random::Player_Random (const PlayerID playerID)
{
    m_playerID = playerID;
}

void Player_Random::getMove(const GameState & state, Move & move)
{
    GameState currentState(state);
    std::vector<Action> legalActions;

    Action endPhase(m_playerID, ActionTypes::END_PHASE, 0);

    while (currentState.getActivePlayer() == m_playerID)
    {
        legalActions.clear();
        currentState.generateLegalActions(legalActions);
        
        Action a = legalActions[rand() % legalActions.size()];
        currentState.doAction(a);
        move.addAction(a);

        // if the move we generated was to undo chill, we must insert an end phase in order to breakthrough
        if (a.getType() == ActionTypes::UNDO_CHILL)
        {
            PRISMATA_ASSERT(currentState.isLegal(endPhase), "We should be able to breakthrough after undo chill");
            currentState.doAction(endPhase);
            move.addAction(endPhase);
        }
    }
}
