#pragma once

#include "Common.h"
#include "GameState.h"
#include "Heuristics.h"

namespace Prismata
{
 
class AttackDamageScenario 
{
    PlayerID                    _attackingPlayerID;
    PlayerID                    _defendingPlayerID;
    GameState                   _initialState;
    GameState                   _beginDefenseState;
    GameState                   _beginBreachState;
    HealthType                  _totalEnemyDefense;
    HealthType                  _totalEnemyHP;
    std::vector<double>         _enemyDefenseLoss;
    std::vector<bool>           _enemyDefenseLossCalculated;
    HealthType                  _maxAttackPotential;
    double                      _wipeoutLoss;
    double                      _totalEnemyLoss;
    bool                        _totalEnemyLossCalculated;
    bool                        _wipeoutLossCalculated;
    bool                        _beginDefenseStateCalculated;
    bool                        _beginBreachStateCalculated;
    bool                        _solved;
    bool                        _print;

    void        calculateBeginBreachState();
    void        calculateBeginDefenseState();

public:
    
    AttackDamageScenario();
    AttackDamageScenario(const GameState & state);

    void        calculateAllEnemyDefenseLoss();
    HealthType  calculateLossDecreaseThreshold(const HealthType currentAttack);
    double      getDefenseLoss(HealthType attack);
    double      getBreachLoss(const HealthType breachAmount);
    double      getWipeoutLoss();
    bool        isIsomorphic(const GameState & state) const;
};
}