#include "PartialPlayer_ActionBuy_TechHeuristic.h"

using namespace Prismata;

PartialPlayer_ActionBuy_TechHeuristic::PartialPlayer_ActionBuy_TechHeuristic(const PlayerID playerID, const size_t & heuristicType)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
    _heuristicType = heuristicType;
}

void PartialPlayer_ActionBuy_TechHeuristic::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    // if we don't have at least 4 gold we can't buy anything so return
    // put this check here since this player will be called often with little gold as the safeguard to not hoarding cash
    if (state.getResources(_playerID).amountOf(Resources::Gold) < 4)
    {
        return;
    }
    
    // call the correct function based on the heuristic we passed into the player
    switch (_heuristicType)
    {
        case TechHeuristics::ELYOT_FORMULA_PLAYOUT:
        case TechHeuristics::ELYOT_FORMULA:
        {
            getMovesElyotFormula(state, move, false);
            break;
        }
        case TechHeuristics::ELYOT_FORMULA_BALANCED:
        {
            getMovesElyotFormula(state, move, true);
            break;
        }
        case TechHeuristics::DIVERSIFY:
        {
            getMovesDiversify(state, move);
            break;
        }
        default:
        {
            PRISMATA_ASSERT(false, "Unknown heuristic type: %d", _heuristicType);
        }
    }
}

void PartialPlayer_ActionBuy_TechHeuristic::getMovesElyotFormula(GameState & state, Move & move, bool balanced)
{
    const PlayerID enemy = state.getEnemy(_playerID);
    const CardType conduitType = CardTypes::CardTypeExists("Conduit") ? CardTypes::GetCardType("Conduit") : CardType();
    const CardType blastforgeType = CardTypes::CardTypeExists("Blastforge") ? CardTypes::GetCardType("Blastforge") : CardType();
    const CardType animusType = CardTypes::CardTypeExists("Animus") ? CardTypes::GetCardType("Animus") : CardType();
    const CardType droneType = CardTypes::CardTypeExists("Drone") ? CardTypes::GetCardType("Drone") : CardType();

    // the actions for buying the individual tech types
    const Action buyConduit(_playerID, ActionTypes::BUY, conduitType.getID());
    const Action buyBlastforge(_playerID, ActionTypes::BUY, blastforgeType.getID());
    const Action buyAnimus(_playerID, ActionTypes::BUY, animusType.getID());

    // variables which will store whether or not certain tech types are legal to buy
    // these will be modified throughout the function based on buy limits, etc
    bool hasConduit         = state.isBuyable(_playerID, conduitType);
    bool hasBlastforge      = state.isBuyable(_playerID, conduitType);
    bool hasAnimus          = state.isBuyable(_playerID, conduitType);
    bool buyableConduit     = state.isLegal(buyConduit);
    bool buyableBlastforge  = state.isLegal(buyBlastforge);
    bool buyableAnimus      = state.isLegal(buyAnimus);
    
    // the following code block is a hard-coded heuristics that prevents us from buying a turn 2 blastforge
    // it checks to see if we have less than 11 gold on the turn and makes blastforge illegal if so
    if (CardTypes::CardTypeExists("Drone") && state.isBuyable(_playerID, droneType))
    {
        size_t totalGoldSpent = 0;
        for (size_t m(0); m < move.size(); ++m)
        {
            const Action & a = move.getAction(m);
            if (a.getType() == ActionTypes::BUY)
            {
                totalGoldSpent += CardTypes::GetAllCardTypes()[a.getID()].getBuyCost().amountOf(Resources::Gold);
            }
        }

        bool enemyHasAttacker = false;
        for (const auto & cardID : state.getCardIDs(enemy))
        {
            const Card & enemyCard = state.getCardByID(cardID);
            if (enemyCard.getType().getAttack() > 0)
            {
                enemyHasAttacker = true;
                break;
            }
        }
    
        size_t totalGold = totalGoldSpent + state.getResources(_playerID).amountOf(Resources::Gold);
        if (!enemyHasAttacker && totalGold < 11)
        {
            buyableBlastforge = false;
        }
        if (totalGold < 10)
        {
            buyableConduit = false;
        }
        if (!enemyHasAttacker && totalGold < 9)
        {
            buyableAnimus = false;
        }
    }
    
    // if we can't buy any tech then stop the calculations here
    if (!buyableConduit && !buyableAnimus && !buyableBlastforge)
    {
        return;
    }

    // determine our current resource income from begin turn producers (ignoring other sources)
    Resources beginTurnIncome;
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Resources & produced = state.getCardByID(cardID).getType().getBeginOwnTurnScript().getEffect().getReceive();

        beginTurnIncome.add(produced);
    }

    // determine the RGB counts of units in the dominion set
    CardID numDominionCards = 0;
    Resources dominionManaCostCount;
    Resources dominionManaCostMax;
    CardID haveEnoughBlastforge = 0;
    CardID needOneMoreBlastforge = 0;
    CardID needTwoMoreBlastforge = 0;
    CardID haveEnoughAnimus = 0;
    CardID needOneMoreAnimus = 0;
    CardID needTwoMoreAnimus = 0;
    for (size_t c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardBuyable & cb = state.getCardBuyableByIndex(c);

        if (!cb.getType().isBaseSet() && cb.getSupplyRemaining(_playerID) > 0)
        {
            numDominionCards++;
            const Resources & cost = cb.getType().getBuyCost();

            dominionManaCostCount.add(cost);

            dominionManaCostMax.set(Resources::Red,   std::max(dominionManaCostMax.amountOf(Resources::Red),   cost.amountOf(Resources::Red)));
            dominionManaCostMax.set(Resources::Blue,  std::max(dominionManaCostMax.amountOf(Resources::Blue),  cost.amountOf(Resources::Blue)));
            dominionManaCostMax.set(Resources::Green, std::max(dominionManaCostMax.amountOf(Resources::Green), cost.amountOf(Resources::Green)));

            int neededBlue = cost.amountOf(Resources::Blue) - beginTurnIncome.amountOf(Resources::Blue);
            int neededRed = cost.amountOf(Resources::Red) - beginTurnIncome.amountOf(Resources::Red);

            if (neededBlue > 0)
            {
                needOneMoreBlastforge++;

                if (neededBlue > 1)
                {
                    needTwoMoreBlastforge++;
                }
            }
            else if (cost.amountOf(Resources::Blue) > 0)
            {
                haveEnoughBlastforge++;
            }

            if (neededRed > 0)
            {
                needOneMoreAnimus++;

                if (neededRed > 2)
                {
                    needTwoMoreAnimus++;
                }
            }
            else if (cost.amountOf(Resources::Red) > 0)
            {
                haveEnoughAnimus++;
            }
        }
    }
    
    // determine the max of each tech producer to buy
    size_t maxConduits      = std::max((ResourceType)2, std::min((ResourceType)3, dominionManaCostMax.amountOf(Resources::Green)));
    size_t maxBlastforge    = std::max((ResourceType)2, dominionManaCostMax.amountOf(Resources::Blue));
    size_t maxAnimus        = std::max((ResourceType)2, dominionManaCostMax.amountOf(Resources::Red));

    // if we are in a situation where only a single tech type is buyable by the player, set the limit to 4
    maxConduits             = (hasBlastforge || hasAnimus)  ? maxConduits   : 4;
    maxBlastforge           = (hasConduit    || hasAnimus)  ? maxBlastforge : 4;
    maxAnimus               = (hasBlastforge || hasConduit) ? maxAnimus     : 4;

    // min each of these with the buy limit if it exists
    maxConduits             = _buyLimits.hasLimit(conduitType) ? std::min(maxConduits, _buyLimits.getLimit(conduitType)) : maxConduits;
    maxBlastforge           = _buyLimits.hasLimit(blastforgeType) ? std::min(maxBlastforge, _buyLimits.getLimit(blastforgeType)) : maxBlastforge;
    maxAnimus               = _buyLimits.hasLimit(animusType) ? std::min(maxAnimus, 2 * _buyLimits.getLimit(animusType)) : maxAnimus;

    PRISMATA_ASSERT(maxAnimus <= 4, "Need more red than 2 animus can produce: %d", maxAnimus);

    // re-determine whether each type is buyable based on our income
    ResourceType gIncome  = beginTurnIncome.amountOf(Resources::Green);
    ResourceType bIncome  = beginTurnIncome.amountOf(Resources::Blue);
    ResourceType rIncome  = beginTurnIncome.amountOf(Resources::Red);

    // if this is the playout version of the tech heuristic then stop buying tech after we have 3 income total
    if (_heuristicType == TechHeuristics::ELYOT_FORMULA_PLAYOUT)
    {
        if (bIncome + gIncome + rIncome / 2 >= 3)
        {
            return;
        }
    }

    buyableConduit    = buyableConduit      && (gIncome < maxConduits);
    buyableBlastforge = buyableBlastforge   && (bIncome < maxBlastforge);
    buyableAnimus     = buyableAnimus       && (rIncome < maxAnimus);

    // if we are concerned with balancing resources, re-calculate legal buying
    if (balanced)
    {
        if ((gIncome > 0) && (rIncome == 0 || bIncome == 0))
        {
            buyableConduit = false;
        }

        if ((bIncome > 0) && (gIncome == 0 || rIncome == 0))
        {
            buyableBlastforge = false;
        }

        if ((rIncome > 0) && (bIncome == 0 || gIncome == 0))
        {
            buyableAnimus = false;
        }   
    }

    ResourceType totalDominionManaCount = dominionManaCostCount.amountOf(Resources::Green) 
                                      + dominionManaCostCount.amountOf(Resources::Blue) 
                                      + dominionManaCostCount.amountOf(Resources::Red);

    // calculate desirabilities based on Elyot's heuristics
    // C_desirability = 3*(1 + total number of green cost symbols in all random set units 
    //                  - #green stored at end of last turn)/(3 + total number of R/G/B cost symbols in all random set units)
    // B_desirability = 2*((can you produce B yet ? 1 : 2) 
    //                  + 2.5*number of units that you need at least one more Blastforge to buy 
    //                  - 1.0*number of units that you need at least 2 more Blastforges to buy 
    //                  + 1.0*total number of units that you already make enough blues every turn to buy)/(3 + number of random set units)
    // A_desirability = 2*((can you produce R yet ? 1 : 2) 
    //                  + 2.5*number of units that you need at least one more Animus to buy 
    //                  - 1.0*number of units that you need at least 2 more Animuses to buy 
    //                  + 1.0*total number of red units that you already make enough reds every turn to buy)/(3 + number of random set units)

    int greenPreviouslyStored = std::max(0, state.getResources(_playerID).amountOf(Resources::Green) - beginTurnIncome.amountOf(Resources::Green));
    
    //double greenDesirability = 3.0 * (1 + dominionManaCostCount.amountOf(Resources::Green) - greenPreviouslyStored)/(3.0 + totalDominionManaCount);
    double greenDesirability = 0;
    if (buyableConduit)
    {
        greenDesirability = 3.0 * (1 + dominionManaCostCount.amountOf(Resources::Green) - greenPreviouslyStored - beginTurnIncome.amountOf(Resources::Green))/(3.0 + totalDominionManaCount);
    }
    
    double blueDesirability = 0;
    if (buyableBlastforge)
    {   
        blueDesirability = 2.0 * ((beginTurnIncome.amountOf(Resources::Blue) > 0 ? 1 : 2) 
                         + 2.5*(needOneMoreBlastforge) 
                         - 1.0*(needTwoMoreBlastforge) 
                         + 1.0*(haveEnoughBlastforge))/(3 + numDominionCards);
    }

    double redDesirability = 0;
    if (buyableAnimus)
    {
        redDesirability = 2.0 * ((beginTurnIncome.amountOf(Resources::Red) > 0 ? 1 : 2) 
                        + 2.5*(needOneMoreAnimus) 
                        - 1.0*(needTwoMoreAnimus) 
                        + 1.0*(haveEnoughAnimus))/(3 + numDominionCards);
    }
    
    double blueVal[3]   = { buyableBlastforge ? 1.0 : 0.0, blueDesirability,  -1.0*beginTurnIncome.amountOf(Resources::Blue) };
    double greenVal[3]  = { buyableConduit ?    1.0 : 0.0, greenDesirability, -1.0*beginTurnIncome.amountOf(Resources::Green) };
    double redVal[3]    = { buyableAnimus ?     1.0 : 0.0, redDesirability,   -1.0*beginTurnIncome.amountOf(Resources::Red) };

    if (buyableBlastforge && greaterThan(blueVal, greenVal, 3) && greaterThan(blueVal, redVal, 3))
    {
        PRISMATA_ASSERT(state.isLegal(buyBlastforge), "Should be able to buy Blastforge");
        state.doAction(buyBlastforge);
        move.addAction(buyBlastforge);
    }
    else if (buyableConduit && greaterThan(greenVal, blueVal, 3) && greaterThan(greenVal, redVal, 3))
    {
        PRISMATA_ASSERT(state.isLegal(buyConduit), "Should be able to buy Blastforge");
        state.doAction(buyConduit);
        move.addAction(buyConduit);
    }
    else if (buyableAnimus && greaterThan(redVal, greenVal, 3) && greaterThan(redVal, blueVal, 3))
    {
        PRISMATA_ASSERT(state.isLegal(buyAnimus), "Should be able to buy Blastforge");
        state.doAction(buyAnimus);
        move.addAction(buyAnimus);
    }

    //printf("Green: %lf, Blue: %lf, Red: %lf\n", greenDesirability, blueDesirability, redDesirability);
}

void PartialPlayer_ActionBuy_TechHeuristic::getMovesDiversify(GameState & state, Move & move)
{

}

bool PartialPlayer_ActionBuy_TechHeuristic::greaterThan(double * v1, double * v2, size_t size)
{
    for (size_t i(0); i < size; ++i)
    {
        if (v1[i] > v2[i])
        {
            return true;
        }

        if (v1[i] < v2[i])
        {
            return false;
        }
    }

    return true;
}