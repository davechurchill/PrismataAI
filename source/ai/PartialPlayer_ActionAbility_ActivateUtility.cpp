#include "PartialPlayer_ActionAbility_ActivateUtility.h"
#include "Heuristics.h"

using namespace Prismata;

PartialPlayer_ActionAbility_ActivateUtility::PartialPlayer_ActionAbility_ActivateUtility(const PlayerID playerID, const CardFilter & filter)
    : _filter(filter)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_ActivateUtility::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    _utility.clear();
    getUtility(state, _utility);

    // sort the cards so that they get activated in an order which is logically and aesthetically correct
    std::sort(std::begin(_utility), std::end(_utility), Heuristics::CardActivateOrderComparator(state));
    

    bool enemyHasApollo = CardTypes::CardTypeExists("Apollo") ? state.numCardsOfType(state.getInactivePlayer(), CardTypes::GetCardType("Apollo")) > 0 : false;

    // activate all our utility cards
    while (true)
    {
        bool cardActivated = false;

        for (const auto & utilityID : _utility)
        {
            const Card & card = state.getCardByID(utilityID);

            // don't activate cards in the filter
            if (_filter[card.getType()])
            {
                continue;
            }

            // don't activate cards that will bring <= our lower bound
            HealthType healthLowerBound = enemyHasApollo ? 3 : 0;
            if ((card.getType().getHealthUsed() > 0) && (card.currentHealth() > healthLowerBound) && (card.currentHealth() - card.getType().getHealthUsed() <= healthLowerBound))
            {
                continue;
            }

            Action a(_playerID, ActionTypes::USE_ABILITY, card.getID());

            if (state.isLegal(a))
            {
                state.doAction(a);
                move.addAction(a);
                cardActivated = true;
            }
        }

        if (!cardActivated)
        {
            break;
        }
    }
}

bool PartialPlayer_ActionAbility_ActivateUtility::isUtilityCard(const Card & card) const
{
    const CardType type = card.getType();

    // has to have an ability
    if (!type.hasAbility())
    {
        return false;
    }

    if (type.hasTargetAbility())
    {
        return false;
    }

    // this is an attacker
    if (type.getAbilityScript().getEffect().getAttackValue() > 0)
    {
        return false;
    }

    // this is an economy card
    if (type.getAbilityScript().getEffect().getReceive().amountOf(Resources::Gold) > 0)
    {
        return false;
    }

    return true;
}

void PartialPlayer_ActionAbility_ActivateUtility::getUtility(const GameState & state, std::vector<CardID> & utility)
{
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        if (isUtilityCard(state.getCardByID(cardID)))
        {
            utility.push_back(cardID);
        }
    }
}