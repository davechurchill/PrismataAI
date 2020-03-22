#include "PartialPlayer_ActionAbility_EconomyDefault.h"

using namespace Prismata;

PartialPlayer_ActionAbility_EconomyDefault::PartialPlayer_ActionAbility_EconomyDefault(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_EconomyDefault::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    std::vector<CardID> econCardIDs;
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);

        // don't activate attackers in this script
        if (!card.getType().isEconCard())
        {
            continue;
        }

        econCardIDs.push_back(cardID);
    }

    for (const auto & cardID : econCardIDs)
    {
        Action a(_playerID, ActionTypes::USE_ABILITY, cardID);
        a.setShift(true);

        // don't need to check legal here because econ cards are defined as having no cost
        if (state.getCardByID(cardID).canUseAbility())
        {
            state.doAction(a);
            move.addAction(a);
        }
    }
}
