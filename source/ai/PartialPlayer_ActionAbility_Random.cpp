#include "PartialPlayer_ActionAbility_Random.h"
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

PartialPlayer_ActionAbility_Random::PartialPlayer_ActionAbility_Random(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_Random::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::vector<Action> legalActions;
    while (state.getActivePlayer() == _playerID && state.getActivePhase() == Phases::Action)
    {
        legalActions.clear();
        state.generateLegalActions(legalActions);
        removeUndoUseAbilityIfProgressExists(legalActions);

        Action a = legalActions[Random::Int(legalActions.size())];
        state.doAction(a);
        move.addAction(a);
    }
}
