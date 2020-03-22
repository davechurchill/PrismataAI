#include "PartialPlayer_ActionAbility_AvoidEconomyWaste.h"
#include "AITools.h"

using namespace Prismata;

PartialPlayer_ActionAbility_AvoidEconomyWaste::PartialPlayer_ActionAbility_AvoidEconomyWaste(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
    _toActivate.reserve(20);
    _toUndo.reserve(20);
}

void PartialPlayer_ActionAbility_AvoidEconomyWaste::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    _toActivate.clear();
    _toUndo.clear();
    
    // avoid wasting resources by ensuring we spend them on abilities here
    // this will usually be called at the end of a given turn
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        if (shouldUseCardAbility(state, cardID))
        {
            _toActivate.push_back(cardID);
        }
    }

    // activate the cards we wanted to activate
    for (const auto & cardID : _toActivate)
    {
        const Action ability(_playerID, ActionTypes::USE_ABILITY, cardID);

        if (state.isLegal(ability))
        {
            state.doAction(ability);
            move.addAction(ability);
        }
    }

    // determine which live cards to undo
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        if (shouldUndoCardAbility(state, cardID))
        {
            _toUndo.push_back(cardID);
        }
    }

    // determine which dead cards to undo
    for (const auto & cardID : state.getKilledCardIDs(_playerID))
    {
        if (shouldUndoCardAbility(state, cardID))
        {
            _toUndo.push_back(cardID);
        }
    }

    // do the undos if they are still legal
    for (const auto & cardID : _toUndo)
    {
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, cardID);
        if (state.isLegal(undoAbility))
        {
            state.doAction(undoAbility);
            move.addAction(undoAbility);
        }
    }
}

bool PartialPlayer_ActionAbility_AvoidEconomyWaste::shouldUseCardAbility(const GameState & state, const CardID cardID) const
{
    const Card & card = state.getCardByID(cardID);
    
    // we aren't concerned with attackers here since that will already have been handled
    // we also don't care about cards that don't have abilities
    // if the card is sac on use it will be handled elsewhere
    if (card.getType().getAbilityScript().getEffect().getAttackValue() > 0 || !card.getType().hasAbility() || card.getType().getAbilityScript().isSelfSac() || card.getType().hasTargetAbility())
    {
        return false;
    }

    // if this is a regular economy card (just taps for gold) skip it, we don't want to re-tap drones we untapped on defense
    if (card.getType().isEconCard())
    {
        return false;
    }

    // if the card costs attack to activate, don't use it here
    if (card.getType().getAbilityScript().getManaCost().amountOf(Resources::Attack) > 0)
    {
        return false;
    }

    // if the card produces only resource which will expire at end of turn then we don't care since it will be wasted anyway
    const Resources & produced = card.getType().getAbilityScript().getEffect().getReceive();
    if (produced.amountOf(Resources::Gold) == 0 && produced.amountOf(Resources::Green) == 0)
    {
        return false;
    }

    return true;
}

bool PartialPlayer_ActionAbility_AvoidEconomyWaste::shouldUndoCardAbility(const GameState & state, const CardID cardID) const
{
    const Card & card = state.getCardByID(cardID);

    // only consider clicked cards in this loop
    if (card.getStatus() != CardStatus::Assigned)
    {
        return false;
    }

    if (card.getType().hasTargetAbility())
    {
        return false;
    }

    const Resources & cost = card.getType().getAbilityScript().getManaCost();
    const Resources & receive = card.getType().getAbilityScript().getEffect().getReceive();

    // don't untap attackers here
    if (receive.amountOf(Resources::Attack) > 0)
    {
        return false;
    }

    // if the ability doesn't cost anything, there's no point to un-click it
    if (cost.empty() && !card.getType().getAbilityScript().isSelfSac())
    {
        return false;
    }

    // don't undo anything that creates something
    if (card.getType().getAbilityScript().getEffect().getCreate().size() > 0)
    {
        return false;
    }

    // the ability cost must not have any disappearing resources
    if (cost.amountOf(Resources::Attack) > 0 || cost.amountOf(Resources::Blue) > 0 || cost.amountOf(Resources::Red) > 0 || cost.amountOf(Resources::Energy) > 0)
    {
        return false;
    }

    // the receive amount must not have any storable resources 
    if (receive.amountOf(Resources::Gold) > 0 || receive.amountOf(Resources::Green) > 0)
    {
        return false;
    }

    // the receive amount must have SOME non-storable resources otherwise there's no point to undo
    if (receive.amountOf(Resources::Attack) == 0 && receive.amountOf(Resources::Blue) == 0 && receive.amountOf(Resources::Red) == 0 && receive.amountOf(Resources::Energy) == 0)
    {
        return false;
    }

    return true;
}