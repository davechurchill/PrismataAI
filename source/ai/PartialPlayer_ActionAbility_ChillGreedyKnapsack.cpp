#include "PartialPlayer_ActionAbility_ChillGreedyKnapsack.h"

using namespace Prismata;

PartialPlayer_ActionAbility_ChillGreedyKnapsack::PartialPlayer_ActionAbility_ChillGreedyKnapsack(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState &))
    : _heuristic(heuristic)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_ChillGreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    // if we don't have any attack, don't chill anything since it won't matter
    if (state.getAttack(_playerID) == 0)
    {
        return;
    }

    const PlayerID enemy = state.getEnemy(_playerID);

    std::vector<Card *> ourChillCards;
    HealthType totalChillAmount = 0;
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.getType().getTargetAbilityType() == ActionTypes::CHILL && card.canUseAbility())
        {
            ourChillCards.push_back(&(Card &)card);
            totalChillAmount += card.getType().getTargetAbilityAmount();
        }
    }

    // if we don't have any chillers, then don't bother
    if (ourChillCards.empty())
    {
        return;
    }

    HealthType chillAmountRemaining = totalChillAmount;

    // make a vector of all enemy blocking cards
    std::vector<Card *> enemyBlockerCards;
    HealthType enemyTotalBlockAmount = 0;

    for (const auto & cardID : state.getCardIDs(enemy))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.canBlock())
        {
            enemyBlockerCards.push_back(&(Card &)card);
            enemyTotalBlockAmount += card.currentHealth();
        }
    }

    // if they have no blockers we can't chill anything
    if (enemyBlockerCards.empty())
    {
        return;
    }
    
    // sort the blockers in the given chill priority
    std::sort(enemyBlockerCards.begin(), enemyBlockerCards.end(), ChillKnapsackCompare(_heuristic, state));

    // figure out the largest blocker remaining if we can chill things perfectly
    HealthType tempChillRemaining = totalChillAmount;
    HealthType largestRemainingEnemyBlocker = 0;
    for (size_t c(0); c < enemyBlockerCards.size(); ++c)
    {
        Card & card(*enemyBlockerCards[c]);

        // if we can chill this card, do it assuming we can do it perfectly
        if (card.currentHealth() <= tempChillRemaining)
        {
            tempChillRemaining -= card.currentHealth();
        }
        // otherwise we can't chill this card, so it's the largest reminaing enemy blocker
        else
        {
            largestRemainingEnemyBlocker = card.currentHealth();
            break;
        }
    }

    // if we won't have enough damage to kill the largest remaining enemy blocker, don't bother chilling anything
    if (state.getAttack(_playerID) < largestRemainingEnemyBlocker)
    {
        return;
    }

    // freeze targets that we can in order
    // this loop is safe because we sorted the units in decreasing health
    for (size_t c(0); c < enemyBlockerCards.size(); ++c)
    {
        Card & card(*enemyBlockerCards[c]);

        // if we can't possibly freeze this unit go to the next
        if ((card.currentChill() >= card.currentHealth()) || (card.currentHealth() > chillAmountRemaining))
        {
            continue;
        }

        // freeze the enemy card and return how much chill we used
        HealthType chillUsed = freezeEnemyCard(card, ourChillCards, state, move);
        
        // a chiller became illegal to use so we have to stop
        if (chillUsed > 0 && chillUsed < card.currentHealth())
        {
            return;
        }

        chillAmountRemaining -= chillUsed;
        enemyTotalBlockAmount -= card.currentHealth();
        
        //std::cout << "Chilled a " << card.getType().getUIName() << std::endl;

        // if we have no chill remaining we should return
        if (chillAmountRemaining == 0)
        {
            return;
        }
    }
}

HealthType PartialPlayer_ActionAbility_ChillGreedyKnapsack::freezeEnemyCard(Card & target, std::vector<Card *> & ourChillCards, GameState & state, Move & move)
{
    HealthType chillUsed = 0;

    // we guarenteed before calling this function that this target is freezeable
    while (target.currentChill() < target.currentHealth())
    {
        HealthType remainingChill = target.currentHealth() - target.currentChill();

        // choose the chiller whose amount is closest to the remaining chill amount
        Card * bestChiller = NULL;
        int bestDiff = 0;

        for (size_t i(0); i < ourChillCards.size(); ++i)
        {
            Card & card = *ourChillCards[i];

            // test to see if the chiller can be activated
            const Action chillActivate(card.getPlayer(), ActionTypes::USE_ABILITY, card.getID());

            if (!state.isLegal(chillActivate))
            {
                continue;
            }

            HealthType chillAmount = card.getType().getTargetAbilityAmount();
            int chillDiff = (int)chillAmount - remainingChill;

            // if we don't have a best chiller yet use this one
            if (!bestChiller)
            {
                bestChiller = &card;
                bestDiff = chillDiff;
                continue;
            }

            // if we find a chiller with the exact remaining amount, use it
            if (chillDiff == 0)
            {
                bestChiller = &card;
                bestDiff = chillDiff;
                break;
            }

            // if the current best is negative
            if (bestDiff < 0)
            {
                // if the current diff is positive, use it
                if (chillDiff > 0)
                {
                    bestChiller = &card;
                    bestDiff = chillDiff;
                }
                // otherwise the current diff is negative only use it if it's closer to 0
                else if (chillDiff > bestDiff)
                {
                    bestChiller = &card;
                    bestDiff = chillDiff;
                }
            }
            // if the current best is positive only consider lower positive numbers
            else if (chillDiff > 0 && chillDiff < bestDiff)
            {
                bestChiller = &card;
                bestDiff = chillDiff;
            }
        }

        // if no chiller is found, then a chiller must have become illegal to click, oh well
        if (bestChiller == NULL)
        {
            return chillUsed;
        }

        Action chillerClick(_playerID, ActionTypes::USE_ABILITY, bestChiller->getID());
        Action chill(_playerID, ActionTypes::CHILL, bestChiller->getID(), target.getID());

        chillUsed += bestChiller->getType().getTargetAbilityAmount();

        PRISMATA_ASSERT(state.isLegal(chillerClick), "This chiller click action should be legal %s %s", bestChiller->toJSONString().c_str(), target.toJSONString().c_str());        
        state.doAction(chillerClick);
        move.addAction(chillerClick);

        PRISMATA_ASSERT(state.isLegal(chill), "This chill action should be legal %s %s", bestChiller->toJSONString().c_str(), target.toJSONString().c_str());
        state.doAction(chill);
        move.addAction(chill);
    }

    return chillUsed;
}

bool PartialPlayer_ActionAbility_ChillGreedyKnapsack::canContinueChilling(const GameState & state, const PlayerID player) const
{
    CardID sum = 0;

    for (const auto & cardID : state.getCardIDs(player))
    {
        const Card & card = state.getCardByID(cardID);

        if (!card.isFrozen() && (card.canBlock() || card.isBreachable()))
        {
            sum++;
            std::cout << card.getID() << " is valid\n";
        }

        // as long as there is one remaining breach card after chilling we are okay
        if (sum > 1)
        {
            return true;
        }
    }

    return false;
}