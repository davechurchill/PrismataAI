#pragma once

#include "Common.h"
#include "GameState.h"

namespace Prismata
{

class HeuristicValues
{
    
    
    std::vector<EvaluationType> _precomputedBuyManaCosts;
    std::vector<EvaluationType> _precomputedBuySacCosts;
    std::vector<EvaluationType> _precomputedBuyTotalCosts;
    std::vector<EvaluationType> _precomputedInflatedManaCostValue;
    std::vector<EvaluationType> _precomputedInflatedTotalCostValue;
    std::vector<EvaluationType> _precomputedInflatedManaCostsGivenToEnemy;
    std::vector<double>         _precomputedInflation;

    HeuristicValues();
    void ResetData();
    void Init();

    // CALCULATION FUNCTIONS USED TO PRECOMPUTE LOOKUP VALUES
    EvaluationType CalculateBuyManaCost(const CardType & type);
    EvaluationType CalculateBuySacCost(const CardType & type);
    EvaluationType CalculateInflatedManaCostGivenToEnemy(const CardType & type);

public:

    static HeuristicValues & Instance();

    EvaluationType GetBuyManaCost(const CardType & type);
    EvaluationType GetBuySacCost(const CardType & type);
    EvaluationType GetBuyTotalCost(const CardType & type);
    EvaluationType GetInflatedManaCostValue(const CardType & type);
    EvaluationType GetInflatedTotalCostValue(const CardType & type);
    EvaluationType GetInflatedManaCostValueGivenToEnemy(const CardType & type);
};

namespace Heuristics
{            
    // SPECIFIC PHASE EVALUATIONS
    EvaluationType SnipeHighestDefense(const Card & card, const GameState & state);
    EvaluationType DefenseHeuristicSaveAttackers(const Card & card, const GameState & state);
    EvaluationType BuyHighestCost(const CardType & type, const GameState & state, const PlayerID player);
    EvaluationType BuyAttackValue(const CardType & type, const GameState & state, const PlayerID player);
    EvaluationType BuyBlockValue(const CardType & type, const GameState & state, const PlayerID player);

    // GENERIC EVALUATIONS
    EvaluationType CurrentCardValue(const Card & blocker, const GameState & state);
    EvaluationType DamageLoss_WillCost(const Card & blocker, const GameState & state, const HealthType & damage);
    EvaluationType DamageLoss_AttackValue(const Card & blocker, const GameState & state, const HealthType & damage);

    // HELPER FUNCTIONS
    HealthType     GetAttackProduced(const CardType & type, const Script & script, const GameState & state, const PlayerID player);
    HealthType     GetAttackProduced(const CardType & type, const GameState & state, const PlayerID player);
    HealthType     GetAttackProduced(const Card & card, const GameState & state, const PlayerID player);


    class CardActivateOrderComparator 
    {
        const GameState & m_state;

    public:

        CardActivateOrderComparator(const GameState & state)
            : m_state(state)
        {
        }

        bool operator() (CardID c1, CardID c2) const;
    };
}
}
