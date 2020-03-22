#include "PartialPlayer_ActionBuy_GreedyKnapsack.h"
#include "AITools.h"
#include <mutex>

using namespace Prismata;

PartialPlayer_ActionBuy_GreedyKnapsack::PartialPlayer_ActionBuy_GreedyKnapsack( const PlayerID playerID, 
                                                                                const CardFilter & filter,
                                                                                EvaluationType (*heuristic)(const CardType &, const GameState &, const PlayerID))
    : _heuristic(heuristic)
    , _filter(filter)
    , _enemyWasChilled(false)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;
    _buyableTypes.reserve(20);
}

void PartialPlayer_ActionBuy_GreedyKnapsack::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    calculateStateData(state);

    // determine the valid buyable card types in this set
    _buyableTypes.clear();

    for (CardID c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardType & cardBuyableType = state.getCardBuyableByIndex(c).getType();

        //std::cout << cardBuyableType.getUIName() << " : " << _heuristic(cardBuyableType, state, _playerID) << "\n";

        if (!shouldNotBuy(cardBuyableType, state))
        {
            _buyableTypes.push_back(cardBuyableType);
        }
    }

    // sort the cards in the vector by the given heurustic
    AITools::SortVector(_buyableTypes, BuyKnapsackCompare(_heuristic, state, _playerID, _enemyAttackPotential));
    //std::sort(_buyableTypes.begin(), _buyableTypes.end(), BuyKnapsackCompare(_heuristic, state, _playerID, _enemyAttackPotential));
    
    // keep buying the best card according to the heuristic
    while (state.getActivePhase() == Phases::Action && !state.isGameOver())
    {
        bool foundBuyable = false;
        for (CardID c(0); c < _buyableTypes.size(); ++c)
        {
            const CardType & cardType = _buyableTypes[c];

            // do another check for whether or not a card should be buyable in case the condition is more dynamic
            if (shouldNotBuy(cardType, state))
            {
                continue;
            }

            // if this card gives attack to the enemy, make sure it won't breach us
            if (cardType.getAttackGivenToEnemy() > 0)
            {
                HealthType newEnemyAttack = _enemyAttackPotential + _enemyChillPotential + cardType.getAttackGivenToEnemy();
                if (newEnemyAttack >= _ourDefense)
                {
                    continue;
                }
            }
                        
            // if everything is legal then buy the card
            Action buyCardAction(_playerID, ActionTypes::BUY, cardType.getID());
            if (state.isLegal(buyCardAction))
            {
                foundBuyable = true;
                state.doAction(buyCardAction);
                move.addAction(buyCardAction);
                
                updateStateData(cardType);

                break;
            }
        }

        if (!foundBuyable)
        {
            break;
        }
    }
}

void PartialPlayer_ActionBuy_GreedyKnapsack::sortBuyables(const GameState & state)
{
    // bubble sort up in this
    for (size_t i(0); i < _buyableTypes.size(); ++i)
    {
        
    }
}

#include "AITools.h"
bool PartialPlayer_ActionBuy_GreedyKnapsack::shouldNotBuy(const CardType & cardType, const GameState & state) const
{
    // we will allow resonate cards even if the buy limit has been exceeded
    bool resonateException = AITools::NumResonatorsReady(cardType, state, _playerID, 1) > 0;
    
    // don't buy it if it's in the filter
    if (_filter.evaluate(state, cardType) && !(resonateException && _filter.getAllowResonate()))
    {
        return true;
    }

    // if we chilled the enemy this turn, don't ruin the calculation by buying something that costs attack
    if (_enemyWasChilled && cardType.getBuyCost().amountOf(Resources::Attack) > 0)
    {
        return true;
    }

    // don't buy out of sync cards
    if (AITools::PurchaseIsOutOfSync(state.getActivePlayer(), cardType, state))
    {
        return true;
    }
    
    // check to see if we have enough of this card already
    CardID numOwned = state.numCardsOfType(state.getActivePlayer(), cardType);
    bool buyLimitExceeded = !resonateException && _buyLimits.hasLimit(cardType.getID()) && (numOwned >= _buyLimits.getLimit(cardType));
    if (buyLimitExceeded)
    {
        return true;
    }

    if (!canAffordToActivate(cardType, state))
    {
        return true;
    }

    return false;
}

void PartialPlayer_ActionBuy_GreedyKnapsack::updateStateData(const CardType & cardTypeBought)
{
    // update if we've given any attack to the enemy
    _enemyAttackPotential += cardTypeBought.getAttackGivenToEnemy();
    
    // add to our defense if the card bought is a prompt blocker
    if (cardTypeBought.isPromptBlocker())
    {
        _ourDefense += cardTypeBought.getStartingHealth();
    }

    // update total attacker cost
    if (hasNonCumulativeManaCostAbility(cardTypeBought))
    {
        _totalAbilityActivateCost.add(cardTypeBought.getAbilityScript().getManaCost());
    }

    // add income generated by the card
    _beginTurnIncome.add(cardTypeBought.getBeginOwnTurnScript().getEffect().getReceive());

    // add income generated by the cards that the card creates (cauterizer, tesla, etc)
    _beginTurnIncome.add(cardTypeBought.getCreatedUnitsManaProduced());
}

void PartialPlayer_ActionBuy_GreedyKnapsack::calculateStateData(const GameState & state)
{
    // determine our total resource income for the state
    _beginTurnIncome = Resources();
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Resources & produced = state.getCardByID(cardID).getType().getBeginOwnTurnScript().getEffect().getReceive();

        _beginTurnIncome.add(produced);
    }

    // determine enemy attack and chill potential for this turn
    _ourDefense = 0;
    _ourDefense = state.getTotalAvailableDefense(_playerID);
    _enemyChillPotential = 0;

    GameState predictedState(state);
    AITools::PredictEnemyNextTurn(predictedState);
    _enemyAttackPotential = predictedState.getResources(state.getEnemy(_playerID)).amountOf(Resources::Attack);

    PlayerID enemy = state.getEnemy(_playerID);
    for (const auto & cardID : predictedState.getCardIDs(enemy))
    {
        const Card & card = predictedState.getCardByID(cardID);

        bool hasTargetAbility = card.getType().hasTargetAbility();
        bool hasChill = card.getType().getTargetAbilityType() == ActionTypes::CHILL;
        bool constr = card.isUnderConstruction();
        bool delayed = card.isDelayed();

        if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::CHILL && !card.isUnderConstruction() && !card.isDelayed())
        {
            _enemyChillPotential += card.getType().getTargetAbilityAmount();
        }
    }
    
    // determine the cost of activating all our attackers
    for (const auto & cardID : state.getCardIDs(_playerID))
    {
        const Card & card = state.getCardByID(cardID);

        if (hasNonCumulativeManaCostAbility(card.getType()))
        {
            _totalAbilityActivateCost.add(card.getType().getAbilityScript().getManaCost());
        }
    }

    // determine if enemy was chilled this turn
    for (const auto & cardID : state.getCardIDs(enemy))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.currentChill() > 0)
        {
            _enemyWasChilled = true;
            break;
        }
    }
}

bool PartialPlayer_ActionBuy_GreedyKnapsack::canAffordToActivate(const CardType & cardType, const GameState & state) const
{
    Resources abilityCost = cardType.getAbilityScript().getManaCost();
    if (abilityCost.empty() || !hasNonCumulativeManaCostAbility(cardType))
    {
        return true;
    }

    // determine out income for the next turn
    Resources beginTurnIncome = _beginTurnIncome;

    // add income generated by the card
    beginTurnIncome.add(cardType.getBeginOwnTurnScript().getEffect().getReceive());

    // add income generated by the cards that the card creates (cauterizer, tesla, etc)
    beginTurnIncome.add(cardType.getCreatedUnitsManaProduced());
     
    Resources totalAbilityCost = _totalAbilityActivateCost;
    totalAbilityCost.add(abilityCost);

    if (!beginTurnIncome.has(totalAbilityCost))
    {
        return false;
    }

    return true;
}

bool PartialPlayer_ActionBuy_GreedyKnapsack::hasNonCumulativeManaCostAbility(const CardType & type) const
{
    const Resources & abilityCost = type.getAbilityScript().getManaCost();

    // ignore cards whose ability costs only green or gold (since cumulative)
    if (    (abilityCost.amountOf(Resources::Energy) == 0) 
            && (abilityCost.amountOf(Resources::Blue) == 0) 
            && (abilityCost.amountOf(Resources::Red) == 0))
    {
        return false;
    }

    return true;
}

bool BuyKnapsackCompare::operator() (const CardType & c1, const CardType & c2) const
{
    HealthType eap = _enemyAttackPotential;
    PRISMATA_ASSERT(c1.getID() < 1000 && c2.getID() < 1000, "CardType ID too high");

    const double frontlinePenalty = 100000;
    EvaluationType h1 = _heuristic(c1, _state, _player);
    EvaluationType h2 = _heuristic(c2, _state, _player);

    if (c1.isFrontline() && _enemyAttackPotential >= c1.getStartingHealth())
    {
        h1 /= frontlinePenalty;
    }

    if (c2.isFrontline() && _enemyAttackPotential >= c2.getStartingHealth())
    {
        h2 /= frontlinePenalty;
    }

    PRISMATA_ASSERT(_enemyAttackPotential == eap, "WOAH!\n");
    return h1 > h2;
}