#include "PartialPlayer_ActionAbility_UntapAvoidBreach.h"
#include "AITools.h"

using namespace Prismata;

PartialPlayer_ActionAbility_UntapAvoidBreach::PartialPlayer_ActionAbility_UntapAvoidBreach(const PlayerID playerID)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_UntapAvoidBreach::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // if we are going to breach the enemy, don't bother doing this at all (Will said)
    if (state.getAttack(_playerID) >= state.getTotalAvailableDefense(state.getEnemy(_playerID)))
    {
        return;
    }

    // put all of our untappable attackers into a vector
    std::vector<CardID> attackingBlockers;
    attackingBlockers.reserve(state.numCards(_playerID));
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);
        const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, cardID);

        HealthType attackProduced = card.getType().getAbilityScript().getEffect().getAttackValue();
        if (card.getType().canBlock(false) && attackProduced > 0 && state.isLegal(undoAbility))
        {
            attackingBlockers.push_back(undoAbility.getID());
        }
    }

    bool avoidedBreach = false;//untapCardsAvoidBreach(state, move, attackingBlockers);

    // if we will still be breached after untapping attackers, untap drones
    if (!avoidedBreach && CardTypes::CardTypeExists("Drone"))
    {
        const CardType & drone = CardTypes::GetCardType("Drone");
        std::vector<CardID> tappedDrones;
        tappedDrones.reserve(20);

        for (const auto & cardID : state.getCardIDs(_playerID))
        {
            const Card & card = state.getCardByID(cardID);
            const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, cardID);

            if (card.getType() == drone && state.isLegal(undoAbility))
            {
                tappedDrones.push_back(card.getID());
            }
        }

        avoidedBreach = untapCardsAvoidBreach(state, move, tappedDrones);
    }

    // TODO: untap things which don't add any value
}

// returns whether or not we avoided being breached
bool PartialPlayer_ActionAbility_UntapAvoidBreach::untapCardsAvoidBreach(GameState & state, Move & move, std::vector<CardID> & cardsToUntap)
{
    bool avoidedBreach = false;
    while (true)
    {
        GameState predictedState(state);
        AITools::PredictEnemyNextTurn(predictedState);

        const HealthType enemyAttackPotential = predictedState.getAttack(state.getEnemy(_playerID));

        if (enemyAttackPotential == 0)
        {
            avoidedBreach = true;
            break;
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

        // if the enemy can't breach us then we don't want to undo attacks
        if (enemyAttackPotential < ourPredictedTotalDefense)
        {
            avoidedBreach = true;
            break;
        }

        // at this point we can still be breached, so choose which card to untap to block with
        CardID cardToUntap = getNextUntapCardID(state, cardsToUntap);

        // if we couldn't find a card to untap then we need to exit
        if (cardToUntap == (CardID)(-1))
        {
            break;
        }

        const Action untap(_playerID, ActionTypes::UNDO_USE_ABILITY, cardToUntap);

        PRISMATA_ASSERT(state.isLegal(untap), "This undo action should be legal");

        move.addAction(untap);
        state.doAction(untap);
    }

    return avoidedBreach;
}

CardID PartialPlayer_ActionAbility_UntapAvoidBreach::getNextUntapCardID(const GameState & state, std::vector<CardID> & cards)
{
    CardID NullID = (CardID)(-1);
    double minADRatio = 100000.0;
    CardID minADIndex = NullID;

    for (CardID c(0); c < cards.size(); ++c)
    {
        if (cards[c] == NullID)
        {
            continue;
        }

        const Action untap(_playerID, ActionTypes::UNDO_USE_ABILITY, cards[c]);

        if (!state.isLegal(untap))
        {
            continue;
        }

        // create a temp card and undo its ability so we can see how much defense and attack it will have when untapped
        Card tempCard = state.getCardByID(cards[c]);
        tempCard.undoUseAbility();

        PRISMATA_ASSERT(tempCard.currentHealth() > 0, "Temp card has zero health");

        double adRatio = (double)tempCard.getType().getAbilityScript().getEffect().getAttackValue() / (double)tempCard.currentHealth();

        if (adRatio < minADRatio)
        {
            minADRatio = adRatio;
            minADIndex = c;
        }
    }

    CardID toReturn = (minADIndex == NullID) ? NullID : cards[minADIndex];

    if (minADIndex != NullID)
    {
        cards[minADIndex] = NullID;
    }

    return toReturn;
}