#include "Player_Random.h"
#include "Random.h"

using namespace Prismata;

namespace
{
    void removeUndoUseAbilityIfProgressExists(std::vector<Action> & actions)
    {
        if (actions.size() <= 1)
        {
            return;
        }

        std::vector<Action> progressActions;
        progressActions.reserve(actions.size());

        for (size_t i(0); i<actions.size(); ++i)
        {
            if (actions[i].getType() != ActionTypes::UNDO_USE_ABILITY)
            {
                progressActions.push_back(actions[i]);
            }
        }

        if (!progressActions.empty())
        {
            actions.swap(progressActions);
        }
    }
}

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
        removeUndoUseAbilityIfProgressExists(legalActions);
        
        Action a = legalActions[Random::Int(legalActions.size())];
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
