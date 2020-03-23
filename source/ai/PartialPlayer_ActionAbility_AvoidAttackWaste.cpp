#include "PartialPlayer_ActionAbility_AvoidAttackWaste.h"
#include "AITools.h"
#include "AttackDamageScenario.h"

using namespace Prismata;

PartialPlayer_ActionAbility_AvoidAttackWaste::PartialPlayer_ActionAbility_AvoidAttackWaste(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
 
    // no need to untap anything if we don't have any attack
    if (state.getAttack(_playerID) == 0)
    {
        return;   
    }
    
    if (weWillWinOnThisAttack(state))
    {
        return;
    }

    // put all of our untappable attackers into a vector
    std::vector<CardID> attackingCards;
    attackingCards.reserve(state.numCards(_playerID));
    getUntappableAttackers(state, attackingCards);

    std::vector<CardID> abilityAttackCostCards;
    abilityAttackCostCards.reserve(state.numCards(_playerID));
    getAbilityAttackCostCards(state, abilityAttackCostCards);

    // stop if we don't have any untappable attackers
    if (attackingCards.empty() && abilityAttackCostCards.empty())
    {
        return;
    }

    // perform our heuristic for untapping blockers with lifespan one
    untapLifeSpanOneHeuristic(state, move, attackingCards);
    
    // determine how much attack we need to reach in order for enemy loss to go down
    AttackDamageScenario scenario(state);
    HealthType lossDecreaseAttackThreshold = scenario.calculateLossDecreaseThreshold(state.getAttack(_playerID));

    // untap attacking units until the threshold
    untapAttackingCards(state, move, attackingCards, lossDecreaseAttackThreshold);

    // activate cards whose abilities cost attack that provide resources (militia etc)
    untapAbilityAttackCostCards(state, move, abilityAttackCostCards, lossDecreaseAttackThreshold);
}

bool PartialPlayer_ActionAbility_AvoidAttackWaste::weWillWinOnThisAttack(const GameState & state)
{
    HealthType totalEnemyHP = 0;

    for (auto cardID : state.getCardIDs(state.getInactivePlayer()))
    {
        totalEnemyHP += state.getCardByID(cardID).currentHealth();
    }

    return state.getAttack(state.getActivePlayer()) >= totalEnemyHP;
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::untapAbilityAttackCostCards(GameState & state, Move & move, std::vector<CardID> & attackingCards, const HealthType lossDecreaseAttackThreshold)
{
    // do the untapping to avoid waste
    for (const auto & cardID : attackingCards)
    {
        const Card & cardToUntap = state.getCardByID(cardID);
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, cardToUntap.getID());
        const Action doAbility(_playerID, ActionTypes::USE_ABILITY, cardToUntap.getID());

        if (!state.isLegal(doAbility))
        {
            continue;
        }

        state.doAction(doAbility);

        // if the undo brings us below the threshold attack amount
        if (state.getAttack(_playerID) < lossDecreaseAttackThreshold)
        {
            PRISMATA_ASSERT(state.isLegal(undoAbility), "Should be able to undo the ability here");

            // redo the action and forget about the undo
            state.doAction(undoAbility);
        }
        // otherwise keep the undo action
        else
        {
            move.addAction(doAbility);
        }

        // if we are 1 attack above the threshold, then we can't safely undo anything else
        if (state.getAttack(_playerID) == lossDecreaseAttackThreshold)
        {
            break;
        }
    }
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::untapAttackingCards(GameState & state, Move & move, std::vector<CardID> & attackingCards, const HealthType lossDecreaseAttackThreshold)
{
    // do the untapping to avoid waste
    for (const auto & cardID : attackingCards)
    {
        const Card & cardToUntap = state.getCardByID(cardID);
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, cardToUntap.getID());
        const Action redoAbility(_playerID, ActionTypes::USE_ABILITY, cardToUntap.getID());

        if (!state.isLegal(undoAbility))
        {
            continue;
        }

        state.doAction(undoAbility);

        // if the undo brings us below the threshold attack amount
        if (state.getAttack(_playerID) < lossDecreaseAttackThreshold)
        {
            PRISMATA_ASSERT(state.isLegal(redoAbility), "Should be able to redo ability here");

            // redo the action and forget about the undo
            state.doAction(redoAbility);
        }
        // otherwise keep the undo action
        else
        {
            move.addAction(undoAbility);
        }

        // if we are 1 attack above the threshold, then we can't safely undo anything else
        if (state.getAttack(_playerID) == lossDecreaseAttackThreshold)
        {
            break;
        }
    }
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::untapLifeSpanOneHeuristic(GameState & state, Move & move, std::vector<CardID> & attackingCards)
{
    // check to see if we have an untappable lifespan 1 unit
    bool haveLifespanOneUnit = false;
    for (size_t i(0); i < attackingCards.size(); ++i)
    {
        if (state.getCardByID(attackingCards[i]).getCurrentLifespan() == 1)
        {
            haveLifespanOneUnit = true;
            break;
        }
    }

    // if we do, we need to check if we should untap it
    if (haveLifespanOneUnit)
    {
        GameState predictedState(state);
        AITools::PredictEnemyNextTurn(predictedState);
        HealthType enemyPredictedAttack = predictedState.getAttack(state.getEnemy(_playerID));

        HealthType ourAvailableDefense = state.getTotalAvailableDefense(_playerID);

        // find our largest absorber
        HealthType ourLargestAbsorber = 0;
        for (const auto & cardID : state.getCardIDs(_playerID))
        {
            const Card & card = state.getCardByID(cardID);

            if (card.canBlock() && !card.getType().isFragile())
            {
                ourLargestAbsorber = std::max(ourLargestAbsorber, card.currentHealth());
            }
        }

        // first go through and untap lifespan one attackers if our heuristic is met
        for (const auto & cardID : attackingCards)
        {
            const Card & card = state.getCardByID(cardID);
            const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());
        
            if (card.getCurrentLifespan() != 1)
            {
                continue;
            }

            if (card.getType().getAbilityScript().getEffect().getAttackValue() > card.currentHealth())
            {
                continue;
            }

            if (enemyPredictedAttack >= (card.currentHealth() + ourLargestAbsorber - 1))
            {
                if (state.isLegal(undoAbility))
                {
                    state.doAction(undoAbility);
                    move.addAction(undoAbility);
                }
            }
        }
    }
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::getAbilityAttackCostCards(const GameState & state, std::vector<CardID> & cards)
{
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        const Action doAbility(_playerID, ActionTypes::USE_ABILITY, card.getID());

        if (card.getType().getAbilityScript().isSelfSac() || card.getType().usesCharges() || card.getType().hasTargetAbility())
        {
            continue;
        }

        if (card.getType().getAbilityScript().getManaCost().amountOf(Resources::Attack) > 0 && state.isLegal(doAbility))
        {
            cards.push_back(card.getID());
        }
    }
}

void PartialPlayer_ActionAbility_AvoidAttackWaste::getUntappableAttackers(const GameState & state, std::vector<CardID> & untappable)
{
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());

        if (card.getType().getAbilityScript().getEffect().getAttackValue() > 0 && state.isLegal(undoAbility))
        {
            untappable.push_back(card.getID());
        }
    }

    // go through dead cards too, they could have been sacced this turn to attack
    for (const auto & cardID : state.getKilledCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, card.getID());

        if (card.getType().getAbilityScript().getEffect().getAttackValue() > 0 && state.isLegal(undoAbility))
        {
            untappable.push_back(card.getID());
        }
    }

    std::sort(untappable.begin(), untappable.end(), WasteCompare(state));
}

WasteCompare::WasteCompare(const GameState & state)
    : _state(state)
{
}

bool WasteCompare::operator() (CardID c1, CardID c2) const
{
    const Card & card1 = _state.getCardByID(c1);
    const Card & card2 = _state.getCardByID(c2);
        
    PRISMATA_ASSERT(card1.getType().getAbilityScript().getEffect().getAttackValue() > 0, "Card1 to be sorter must be attacker");
    PRISMATA_ASSERT(card2.getType().getAbilityScript().getEffect().getAttackValue() > 0, "Card2 to be sorter must be attacker");

        

    const double c1props[4] = { card1.getType().getAbilityScript().isSelfSac() ? Heuristics::CurrentCardValue(card1, _state) : 0,      // 1. cards which sac themselves (card cost > tiebreak)
                                card1.getType().usesCharges() ? -card1.getCurrentCharges() : -10000.0,            // 2. cards with charges whose ability generates attack (card cost > tiebreak)
                                static_cast<double>(card1.getType().getAbilityScript().getDelay()),                                                      // 3. delayed cards
                                ((double)card1.currentHealth() / (double)card1.getType().getAbilityScript().getEffect().getAttackValue())};     // 4. attackers that can defend (attack/defense ratio tiebreak)

    const double c2props[4] = { card2.getType().getAbilityScript().isSelfSac() ? Heuristics::CurrentCardValue(card2, _state) : 0,      // 1. cards which sac themselves (card cost > tiebreak)
                                card2.getType().usesCharges() ? -card2.getCurrentCharges() : -10000.0,            // 2. cards with charges whose ability generates attack (card cost > tiebreak)
                                static_cast<double>(card2.getType().getAbilityScript().getDelay()),                                                      // 3. delayed cards
                                ((double)card2.currentHealth() / (double)card2.getType().getAbilityScript().getEffect().getAttackValue())};     // 4. attackers that can defend (attack/defense ratio tiebreak)

    for (size_t i(0); i < 4; ++i)
    {
        if (c1props[i] > c2props[i])
        {
            return true;
        }

        if (c1props[i] < c2props[i])
        {
            return false;
        }
    }

    return c1 < c2;
}