#include "PartialPlayer_ActionBuy_AvoidBreach.h"

using namespace Prismata;

PartialPlayer_ActionBuy_AvoidBreach::PartialPlayer_ActionBuy_AvoidBreach(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
}

#include "AITools.h"
void PartialPlayer_ActionBuy_AvoidBreach::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // calculate a rough upper bound of enemy attack potential first
    HealthType enemyAttackUpperBound = 0;
    for (const auto & cardID : state.getCardIDs(state.getInactivePlayer()))
    {
        const Card & card = state.getCardByID(cardID);

        // only count cards that will be ready next turn
        if (card.getConstructionTime() < 2)
        {
            enemyAttackUpperBound += card.getType().getAttack();
        }
    }

    if (enemyAttackUpperBound == 0)
    {
        return;
    }
    
    // estimate how much attack our opponent can send at us next turn
    GameState predictedState(state);
    AITools::PredictEnemyNextTurn(predictedState);
    HealthType enemyAttackPotential = predictedState.getAttack(state.getEnemy(_playerID));

    if (enemyAttackPotential == 0)
    {
        return;
    }

    // calculate how much chill the enemy has
    HealthType enemyChillPotential = 0;
    PlayerID enemy = predictedState.getEnemy(_playerID);
    for (const auto & cardID : predictedState.getCardIDs(enemy))
    {
        const Card & card = predictedState.getCardByID(cardID);

        bool hasTargetAbility = card.getType().hasTargetAbility();
        bool hasChill = card.getType().getTargetAbilityType() == ActionTypes::CHILL;
        bool constr = card.isUnderConstruction();
        bool delayed = card.isDelayed();

        if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::CHILL && !card.isUnderConstruction() && !card.isDelayed())
        {
            enemyChillPotential += card.getType().getTargetAbilityAmount();
        }
    }

    // only worry about chill if they also have attack
    if (enemyAttackPotential > 0)
    {
        enemyAttackPotential += enemyChillPotential;
    }

    // calculate our predicted available defense
    HealthType ourPredictedTotalDefense = 0;
    HealthType ourPredictedBiggestBlocker = 0;
    for (const auto & cardID : predictedState.getCardIDs(_playerID))
    {
        const Card & card = predictedState.getCardByID(cardID);

        if (card.canBlock())
        {
            ourPredictedTotalDefense += card.currentHealth();
        }

        if (card.canBlock() && !card.getType().isFragile())
        {
            ourPredictedBiggestBlocker = std::max(ourPredictedBiggestBlocker, card.currentHealth());
        }
    }

    // if we can absorb everything with a single blocker then we don't need to buy
    if (enemyAttackPotential < ourPredictedBiggestBlocker)
    {
        return;
    }
    
    // construct a vector of all prompt blocking types that we can buy
    std::vector<CardType> promptBlockers;
    for (CardID c(0); c<state.numCardsBuyable(); ++c)
    {
        const CardBuyable & cardBuyable = predictedState.getCardBuyableByIndex(c);
        const CardType cardType = cardBuyable.getType();

        if (cardType.isPromptBlocker())
        {
            promptBlockers.push_back(cardType);
        }
    }

    // if we can't buy any prompt blockers then this algorithm is kinda worthless
    if (promptBlockers.empty())
    {
        return;
    }

    // sort the prompt blockers by defense value (we want to buy large defense first)
    std::sort(promptBlockers.begin(), promptBlockers.end(), CompareMostDefense());
    
    // if we can buy a single prompt blocker to absorb all the damage, buy the cheapest one
    bool absorberFound = false;
    CardType cheapestAbsorber;
    for (CardID c(0); c < promptBlockers.size(); ++c)
    {
        const CardType blockerType = promptBlockers[c];
        Action buyCard(_playerID, ActionTypes::BUY, blockerType.getID());
        
        // candidate absorber test
        if ((blockerType.getStartingHealth() > enemyAttackPotential) && !blockerType.isFragile() && !(blockerType.getLifespan() == 1) && state.isLegal(buyCard))
        {
            if (!absorberFound || HeuristicValues::Instance().GetBuyTotalCost(blockerType) < HeuristicValues::Instance().GetBuyTotalCost(cheapestAbsorber))
            {
                absorberFound = true;
                cheapestAbsorber = blockerType;
            }
            
        }
    }

    // if we found a single absorber, buy it
    if (absorberFound)
    {
        Action buyAbsorber(_playerID, ActionTypes::BUY, cheapestAbsorber.getID());
        move.addAction(buyAbsorber);
        state.doAction(buyAbsorber);
        return;
    }

    // otherwise if we won't get breached, don't buy anything
    if (enemyAttackPotential < ourPredictedTotalDefense)
    {
        return;
    }

    // keep buying the prompt blockers until we are out of breach range
    HealthType defenseBought = 0;
    for (CardID c(0); c < promptBlockers.size(); ++c)
    {
        Action buyCard(_playerID, ActionTypes::BUY, promptBlockers[c].getID());

        while (state.isLegal(buyCard))
        {
            move.addAction(buyCard);
            state.doAction(buyCard);
            defenseBought += promptBlockers[c].getStartingHealth();

            if (ourPredictedTotalDefense + defenseBought > enemyAttackPotential)
            {
                return;
            }
        }
    }
}
