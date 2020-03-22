#include "AttackDamageScenario.h"
#include "AITools.h"
#include "PartialPlayer_ActionAbility_AttackDefault.h"

using namespace Prismata;

/*
 * AttackDamageScenario class calculates the sum of resource cost loss on defense for a given input attack amount
 * The class's main function is to memoize already computed values so that subsequent lookups require no calculation
 *
 * Class logic flow:
 *   
 *   Constructor      - Precomputes defending player's total defense amount, initializes data structures
 *
 *   getDefenseLoss() - Computes how much loss will be incurred by defender given an input attack amount
 *                    - Caches previously computed values
 *
 *   calculateLossDecreaseThreshold()
 *                    - Given an input attack amount, computes the minimum attack necessary to do the same damage
 *                    - Example: Attack for 5 into 2 Walls, will return 3 - since 3 does the same damage
 *                    - Used mainly for figuring out which attackers to untap 
 */

AttackDamageScenario::AttackDamageScenario()
    : _attackingPlayerID(0)
    , _defendingPlayerID(1)
    , _maxAttackPotential(60)
    , _solved(false)
    , _print(false)
    , _wipeoutLossCalculated(false)
    , _beginDefenseStateCalculated(false)
    , _beginBreachStateCalculated(false)
    , _totalEnemyLossCalculated(false)
    , _wipeoutLoss(0)
    , _totalEnemyHP(0)
    , _totalEnemyLoss(0)
{

}
    
AttackDamageScenario::AttackDamageScenario(const GameState & state)
    : _attackingPlayerID(state.getActivePlayer())
    , _defendingPlayerID(state.getInactivePlayer())
    , _initialState(state)
    , _maxAttackPotential(60) 
    , _solved(false)
    , _print(true)
    , _wipeoutLoss(0)
    , _wipeoutLossCalculated(false)
    , _beginDefenseStateCalculated(false)
    , _beginBreachStateCalculated(false)
    , _totalEnemyLossCalculated(false)
    , _totalEnemyHP(0)
    , _totalEnemyLoss(0)
{
    // assume we've already activated our attackers and we currently have the maximum amount of attack we can achieve
    _totalEnemyDefense = state.getTotalAvailableDefense(_defendingPlayerID);

    for (const auto & cardID : _initialState.getCardIDs(_defendingPlayerID))
    {
        _totalEnemyHP += _initialState.getCardByID(cardID).currentHealth();
    }

    _maxAttackPotential = _totalEnemyHP;
    _enemyDefenseLoss = std::vector<double>(_maxAttackPotential + 1, 0);
    _enemyDefenseLossCalculated = std::vector<bool>(_maxAttackPotential + 1, false);
    _enemyDefenseLoss[0] = 0;
    _enemyDefenseLossCalculated[0] = true;
}

// Given an input attack amount, calculates the minimum attack amount for which damage done stays the same
//   For example, if defender has 2 walls and we input 5 damage, this function will return 3, since
//   attacking for 3 will do the same amount of damage as attacking for 5, but 2 will not.
HealthType AttackDamageScenario::calculateLossDecreaseThreshold(const HealthType currentAttack)
{
    bool foundLossThreshold = false;
    double originalEnemyDefenseLoss = getDefenseLoss(currentAttack);
    HealthType lossDecreaseAttackThreshold = 0;

    for (int attack(currentAttack); attack >= 0; --attack)
    {
        double enemyDefenseLoss = getDefenseLoss(attack);
        lossDecreaseAttackThreshold = attack;
        
        if (enemyDefenseLoss < originalEnemyDefenseLoss)
        {
            foundLossThreshold = true;
            break;
        }
    }

    return lossDecreaseAttackThreshold + (foundLossThreshold ? 1 : 0);
}

double AttackDamageScenario::getDefenseLoss(HealthType attack)
{
    // we never have to worry about the case where there's more attack than total enemy hp, since it's overkill
    if (attack > _totalEnemyHP)
    {
        return getDefenseLoss(_totalEnemyHP);
    }
    
    // if we've already calculated this value, return it
    if (_enemyDefenseLossCalculated[attack])
    {
        return _enemyDefenseLoss[attack];
    }

    double defenseLoss = 0;
    // case of breach for zero is easily calculated
    if (attack == _totalEnemyDefense)
    {
        defenseLoss = getWipeoutLoss();

        if (_totalEnemyDefense == _totalEnemyHP)
        {
            _totalEnemyLoss = defenseLoss;
            _totalEnemyLossCalculated = true;
        }
    }
    // the case of a breach can be calculated separately
    else if (attack > _totalEnemyDefense)
    {
        if (!_beginBreachStateCalculated)
        {
            calculateBeginBreachState();
        }

        // if this will kill every unit the enemy has and we already know that loss, then use it
        if (attack >= _totalEnemyHP && _totalEnemyLossCalculated)
        {
            defenseLoss = _totalEnemyLoss;
        }
        // otherwise we have to manually calculate the breach loss
        else
        {
            PartialPlayer_Breach_GreedyKnapsack breachPartialPlayers[2] = {PartialPlayer_Breach_GreedyKnapsack(0, true), PartialPlayer_Breach_GreedyKnapsack(1, true)};
        
            GameState tempState(_beginBreachState);
            tempState.manuallySetAttack(_attackingPlayerID, attack - _totalEnemyDefense);

            Move m;
            breachPartialPlayers[_attackingPlayerID].getMove(tempState, m);
        
            defenseLoss = getWipeoutLoss() + breachPartialPlayers[_attackingPlayerID].getTotalBreachDamageLoss(); 

            // if we killed every enemy unit, then we know the total loss
            if (tempState.numCards(_defendingPlayerID) == 0)
            {
                _totalEnemyLoss = defenseLoss;
                _totalEnemyLossCalculated = true;
            }
        }
    }
    // the case of enemy getting to block is calculated separately
    else if (attack < _totalEnemyDefense)
    {
        if (!_beginDefenseStateCalculated)
        {
            calculateBeginDefenseState();
        }

        // if the attacking player actually lost the game on this attack by saccing their last unit
        if (_beginDefenseState.numCards(_attackingPlayerID) == 0)
        {
            // we can just say the defense loss is zero since we'll lose the game
            defenseLoss = 0;
        }
        else
        {   
            _beginDefenseState.manuallySetAttack(_attackingPlayerID, attack);
            BlockIterator blockIterator(_beginDefenseState, &Heuristics::DamageLoss_WillCost);
            blockIterator.solve();

            defenseLoss = blockIterator.getMinLossScore();
        }
    }
    
    // deprecated old way of doing it (keep for tests)
    //GameState testState(_initialState);
    //testState.manuallySetAttack(_attackingPlayerID, attack);
    //defenseLoss = AITools::CalculateEnemyNextTurnDefenseLoss(testState);

    _enemyDefenseLoss[attack] = defenseLoss;
    _enemyDefenseLossCalculated[attack] = true;

    return _enemyDefenseLoss[attack];
}

double AttackDamageScenario::getBreachLoss(const HealthType breachAmount)
{
    return getDefenseLoss(_totalEnemyDefense + breachAmount);
}

void AttackDamageScenario::calculateBeginBreachState()
{
    _beginBreachState = _initialState;

    _beginBreachState.manuallySetAttack(_attackingPlayerID, _totalEnemyDefense + 1);

    const int startingPhase = _beginBreachState.getActivePhase();
    const PlayerID player = _beginBreachState.getActivePlayer();
    const PlayerID enemy = _beginBreachState.getInactivePlayer();
    Move moves[2] = {Move(), Move()};
    moves[0].clear();
    moves[1].clear();

    // we start on the active player's turn and assume that we will be sending over the current amount of attack
    const Action endPhase(player, ActionTypes::END_PHASE, 0);

    // if it's the action phase we just want to pass and do nothing else, let the calling function decide how much we've attacked for
    if (_beginBreachState.getActivePhase() == Phases::Action)
    {
        PRISMATA_ASSERT(_beginBreachState.isLegal(endPhase), "We should be able to end here");
        _beginBreachState.doAction(endPhase);
    }

    _beginBreachStateCalculated = true;
    
    PRISMATA_ASSERT(_beginBreachState.getActivePhase() == Phases::Breach || _beginBreachState.numCards(enemy) == 0, "We should be in the breach phase now");
}

void AttackDamageScenario::calculateBeginDefenseState()
{
    _beginDefenseState = _initialState;
    _beginDefenseState.manuallySetAttack(_attackingPlayerID, std::max(0, (int)_totalEnemyDefense - 1));

    const int startingPhase = _beginDefenseState.getActivePhase();
    const PlayerID player = _beginDefenseState.getActivePlayer();
    const PlayerID enemy = _beginDefenseState.getInactivePlayer();
    Move moves[2] = {Move(), Move()};
    moves[0].clear();
    moves[1].clear();

    // we start on the active player's turn and assume that we will be sending over the current amount of attack
    const Action endPhase(player, ActionTypes::END_PHASE, 0);

    // if it's the action phase we just want to pass and do nothing else, let the calling function decide how much we've attacked for
    if (_beginDefenseState.getActivePhase() == Phases::Action)
    {
        PRISMATA_ASSERT(_beginDefenseState.isLegal(endPhase), "We should be able to end here");
        _beginDefenseState.doAction(endPhase);
    }

    PRISMATA_ASSERT(_beginDefenseState.getActivePhase() != Phases::Breach, "We shouldn't be in the breach phase now");
    PRISMATA_ASSERT(_beginDefenseState.getActivePhase() == Phases::Confirm, "We should be at the confirm phase now");

    _beginDefenseState.doAction(endPhase);

    _beginDefenseStateCalculated = true;

    if (_beginDefenseState.isGameOver())
    {
        return;
    }
    
    PRISMATA_ASSERT(_beginDefenseState.getActivePlayer() == enemy, "It should be the enemy's turn now");
    PRISMATA_ASSERT(_beginDefenseState.getActivePhase() == Phases::Defense, "We should be at the defense phase now");
}

double AttackDamageScenario::getWipeoutLoss()
{
    if (_wipeoutLossCalculated)
    {
        return _wipeoutLoss;
    }

    _enemyDefenseLoss[_totalEnemyDefense] = AITools::CalculateWipeoutLoss(_initialState, _defendingPlayerID);
    return _enemyDefenseLoss[_totalEnemyDefense];
}

void AttackDamageScenario::calculateAllEnemyDefenseLoss()
{
    Timer t;
    t.start();

    for (int attack(_maxAttackPotential); attack >= 0; --attack)
    {
        if (_enemyDefenseLossCalculated[attack])
        {
            continue;
        }

        double enemyDefenseLoss = getDefenseLoss(attack);

        if (_print)
        {
            std::cout << "Attack " << attack << ", Loss " << enemyDefenseLoss << "\n";
        }
    }

    double ms = t.getElapsedTimeInMilliSec();
    std::cout << "calculateAllEnemyDefenseLoss took " << ms << "ms\n";
}

bool AttackDamageScenario::isIsomorphic(const GameState & state) const
{
    return _initialState.isPlayerIsomorphic(state, _defendingPlayerID);
}