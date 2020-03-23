#pragma once

#include "Common.h"
#include "GameState.h"

#include "BuyLimits.h"
#include "Heuristics.h"
#include "ChillScenario.h"
#include "AttackDamageScenario.h"

namespace Prismata
{
 
class BuyCost
{
public:
    
    double resourceCost;
    double lossCost;

    BuyCost()
        : resourceCost(0)
        , lossCost(0)
    {
    }

    BuyCost(const double m, const double l)
        : resourceCost(m)
        , lossCost(l)
    {
    }

    bool operator < (const BuyCost & rhs) const
    {
        //double cost = resourceCost + lossCost;
        //double otherCost = rhs.resourceCost + rhs.lossCost;

        //return (cost == otherCost) ? (lossCost < rhs.lossCost) : (cost < otherCost);

        // favour no loss, deprecated
        return (lossCost == rhs.lossCost) ? (resourceCost < rhs.resourceCost) : (lossCost < rhs.lossCost);
    }

    const BuyCost operator + (const BuyCost & rhs) const
    {
        return BuyCost(resourceCost + rhs.resourceCost, lossCost + rhs.lossCost);
    }
};

namespace ChillCalculationMethod
{
    enum { NONE, HEURISTIC, SOLVER};
}

class BreachIteratorParameters
{
   
public:

    size_t  maxUntapDrones;
    size_t  maxChillSolverIterations;
    int     chillCalculationMethod;

    BreachIteratorParameters()
        : maxUntapDrones(0)
        , maxChillSolverIterations(0)
        , chillCalculationMethod(ChillCalculationMethod::NONE)
    {
    
    }

    BreachIteratorParameters(const size_t & drones, const int & method, const size_t & iterations)
        : maxUntapDrones(drones)
        , maxChillSolverIterations(iterations)
        , chillCalculationMethod(method)
    {
    
    }
};

class AvoidBreachBuyIterator 
{
    BreachIteratorParameters            _params;                    
    ChillScenario                       _chillScenario;             
    AttackDamageScenario                _attackDamageScenario;

    GameState                           _state;
    GameState                           _predictedNextTurnState;
    const GameState                     _initialState;
    PlayerID                            _playerID;
    HealthType                          _predictedEnemyAttack;
    HealthType                          _enemyChillPotential;
    HealthType                          _ourAvailableDefense;
    HealthType                          _ourLargestBlocker;
    HealthType                          _enemyMaxBreachPotential;
    HealthType                          _initialFrontlineUsed;
    
    size_t                              _nodesSearched;
    
    double                              _originalBlockLoss;
    double                              _minTrueCost;
    double                              _minSpent;
    BuyCost                             _minSolutionCost;
    BuyCost                             _ourLargestBlockerCost;

    std::vector<CardID>                 _promptCardBuyableIndex;
    std::vector<Action>                 _actionStack;
    std::vector<Action>                 _bestActionSequence;

    CardID                              _numTappedDrones;
    CardID                              _bestUntapDrones;
    CardType                            _droneType;
    CardType                            _doomedDroneType;
    CardType                            _wallType;

    bool                                _droneExists;
    bool                                _wallExists;
    bool                                _doomedDroneExists;
    bool                                _solutionFound;
    bool                                _print;

    void                                setBestSequence(CardID untapDrones);
    void                                updateBestSolution(const GameState & state, BuyCost cost, double defenseBought);
    void                                reverseBuy(GameState & state, const Action & buy);
    void                                checkNewBestSolution(double cost, CardID untapdrones);
    void                                doChillStatistics();
    CardID                              calculateNumUntappableDrones(const GameState & state);
    HealthType                          calculateEnemyChillUsage();

public:
    
    AvoidBreachBuyIterator(const GameState & state, const BreachIteratorParameters & params);
    
    void        solve();
    void        recurse(const CardID currentCardBuyableIndex, const size_t numBought, const HealthType largestAbsorberHealth, const BuyCost largestAbsorberCost, const BuyCost cost, const HealthType defenseBought, const HealthType frontlineAssigned);
    void        setBuyLimits(const BuyLimits & buyLimits);
    void        debugSolve();
    void        getMove(Move & move);
    
    bool        solutionFound() const;
    BuyCost     getCardCost(const CardType type);
    size_t      getNodesSearched();
    
    void        setPrint(bool print);
    void        printStack(const GameState & state, HealthType largestAbsorberHealth, BuyCost largestAbsorberCost, BuyCost cost, HealthType defenseBought, CardID untapDrones,  double spent, double trueCost,HealthType chill);
};

class PromptBlockerCompare 
{
    const GameState & _state;

public:

    PromptBlockerCompare(const GameState & state)
        : _state(state)
    {
    }

    bool operator() (const CardID cardBuyableIndex1, const CardID cardBuyableIndex2) const
    {
        return _state.getCardBuyableByIndex(cardBuyableIndex1).getType().getStartingHealth() > _state.getCardBuyableByIndex(cardBuyableIndex2).getType().getStartingHealth();
    }
};

class DroneBlockCompare 
{
    const GameState & _state;

public:

    DroneBlockCompare(const GameState & state)
        : _state(state)
    {
    }

    bool operator() (const CardID card1, const CardID card2) const
    {
        // we want to sort so that drones > doomed drone and lower lifespan doomed drone < higher lifespan
        // so we set a drone's lifespan to 100 then sort in ascending order
        TurnType lifespan1 = _state.getCardByID(card1).getCurrentLifespan();
        TurnType lifespan2 = _state.getCardByID(card2).getCurrentLifespan();

        if (lifespan1 == 0) { lifespan1 = 100; }
        if (lifespan2 == 0) { lifespan2 = 100; }

        return lifespan1 < lifespan2;
    }
};

}
