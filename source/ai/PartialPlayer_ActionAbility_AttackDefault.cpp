#include "PartialPlayer_ActionAbility_AttackDefault.h"
#include "Heuristics.h"

using namespace Prismata;

PartialPlayer_ActionAbility_AttackDefault::PartialPlayer_ActionAbility_AttackDefault(const PlayerID playerID, const CardFilter & filter)
    : _filter(filter)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_AttackDefault::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    _attackers.clear();
    getAttackers(state, _attackers);

    // sort the cards so that they get activated in an order which is logically and aesthetically correct
    std::sort(std::begin(_attackers), std::end(_attackers), Heuristics::CardActivateOrderComparator(state));

    while (true)
    {
        bool cardActivated = false;

        for (CardID c(0); c < _attackers.size(); ++c)
        {
            const Card & card = state.getCardByID(_attackers[c]);

            // don't activate cards in the filter
            if (_filter[card.getType()])
            {
                continue;
            }

            // don't activate cards which will kill themselves due to HP loss
            if ((card.getType().getHealthUsed() > 0) && (card.currentHealth() == card.getType().getHealthUsed()))
            {
                continue;
            }

            Action a(_playerID, ActionTypes::USE_ABILITY, card.getID());

            if (state.isLegal(a))
            {
                state.doAction(a);
                move.addAction(a);
                cardActivated = true;

                // deprecrated
                // we have to break here in case of iterator invalidation
                // activating a card could cause another one to die
                // break;
            }
        }

        if (!cardActivated)
        {
            break;
        }
    }
}

void PartialPlayer_ActionAbility_AttackDefault::getAttackers(const GameState & state, std::vector<CardID> & attackers)
{
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        if (state.getCardByID(cardID).getType().getAbilityAttackAmount() > 0)
        {
            attackers.push_back(cardID);
        }
    }
}