#include "Heuristics.h"
#include "AITools.h"
#include <vector>

using namespace Prismata;

#define WILL_VALUE_ATTACK 2.25
#define WILL_VALUE_BLUE   1.50
#define WILL_VALUE_GREEN  1.20
#define WILL_VALUE_MONEY  1.00
#define WILL_VALUE_RED    0.90
#define WILL_VALUE_ENERGY 0.50
#define WILL_VALUE_CONSTR 1.28
#define WILL_VALUE_PROMPT 1.13


HeuristicValues::HeuristicValues()
{
    Init();
}

HeuristicValues & HeuristicValues::Instance()
{
    static HeuristicValues instance;
    return instance;
}

void HeuristicValues::ResetData()
{
    _precomputedBuySacCosts                     = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedBuyManaCosts                    = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedBuyTotalCosts                   = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedBuyManaCosts                    = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedInflatedManaCostValue           = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedInflatedTotalCostValue          = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedInflatedManaCostsGivenToEnemy   = std::vector<EvaluationType>(CardTypes::GetAllCardTypes().size(), 0);
    _precomputedInflation                       = std::vector<double>(30, 0);
}


EvaluationType HeuristicValues::GetInflatedManaCostValue(const CardType & type)
{
    return _precomputedInflatedManaCostValue[type.getID()];
}

EvaluationType HeuristicValues::GetInflatedTotalCostValue(const CardType & type)
{
    return _precomputedInflatedTotalCostValue[type.getID()];
}

EvaluationType HeuristicValues::GetBuyManaCost(const CardType & type)
{
    return _precomputedBuyManaCosts[type.getID()];
}

EvaluationType HeuristicValues::GetInflatedManaCostValueGivenToEnemy(const CardType & type)
{
    return _precomputedInflatedManaCostsGivenToEnemy[type.getID()];
}

EvaluationType HeuristicValues::GetBuyTotalCost(const CardType & type)
{
    return _precomputedBuyTotalCosts[type.getID()];
}


void HeuristicValues::Init()
{
    ResetData();

    _precomputedInflation[0] = 1.0 / WILL_VALUE_PROMPT;
    _precomputedInflation[1] = 1.0;
    for (size_t i(2); i < _precomputedInflation.size(); ++i)
    {
        _precomputedInflation[i] = _precomputedInflation[i-1] * WILL_VALUE_CONSTR;
    }

    // calculate all the precomputed values
    // this must be done in this order since later calculations rely on previous ones
    for (const CardType & type : CardTypes::GetAllCardTypes())
    {
        _precomputedBuyManaCosts[type.getID()]              = CalculateBuyManaCost(type);
        _precomputedBuySacCosts[type.getID()]               = CalculateBuySacCost(type);
        _precomputedBuyTotalCosts[type.getID()]             = GetBuyManaCost(type) + GetBuySacCost(type);
        _precomputedInflatedManaCostValue[type.getID()]     = GetBuyManaCost(type) * _precomputedInflation[type.getConstructionTime()];
        _precomputedInflatedTotalCostValue[type.getID()]    = GetBuyTotalCost(type) * _precomputedInflation[type.getConstructionTime()];
    }

    // calculating the given costs must be done after all the others have been done since it relies on them
    for (const CardType & type : CardTypes::GetAllCardTypes())
    {
        _precomputedInflatedManaCostsGivenToEnemy[type.getID()] = CalculateInflatedManaCostGivenToEnemy(type);
    }

    // forcefield's inflated resource cost is slightly less than that of engineer, which leads to some issues on blocking
    // let's just say for now that forcefield does not incur inflation, due to its extra drone cost
    if (CardTypes::CardTypeExists("Forcefield"))
    {
        const CardType & ffType = CardTypes::GetCardType("Forcefield");

        // magic heuristic for forcefield since it's so hard to price. this is approx 2/3 of a wall
        _precomputedInflatedManaCostValue[ffType.getID()] = 3.75; //_precomputedBuyManaCosts[ffType.getID()];
        _precomputedInflatedTotalCostValue[ffType.getID()] = _precomputedBuyTotalCosts[ffType.getID()];
    }
}

EvaluationType HeuristicValues::CalculateBuySacCost(const CardType & type)
{
    double sacCost = 0;

    for (const SacDescription & sacDescription : type.getBuySac())
    {
        sacCost += sacDescription.getMultiple() * CalculateBuyManaCost(CardTypes::GetAllCardTypes()[sacDescription.getTypeID()]);
    }

    return sacCost;
}

EvaluationType HeuristicValues::GetBuySacCost(const CardType & type)
{
    return _precomputedBuySacCosts[type.getID()];
}

EvaluationType HeuristicValues::CalculateBuyManaCost(const CardType & type)
{
    EvaluationType scaledCost = 0;
    
    scaledCost += type.getBuyCost().amountOf(Resources::Gold)   * WILL_VALUE_MONEY;
    scaledCost += type.getBuyCost().amountOf(Resources::Blue)   * WILL_VALUE_BLUE;
    scaledCost += type.getBuyCost().amountOf(Resources::Energy) * WILL_VALUE_ENERGY;
    scaledCost += type.getBuyCost().amountOf(Resources::Green)  * WILL_VALUE_GREEN;
    scaledCost += type.getBuyCost().amountOf(Resources::Red)    * WILL_VALUE_RED;
    scaledCost += type.getBuyCost().amountOf(Resources::Attack) * WILL_VALUE_ATTACK;

    return scaledCost;
}

EvaluationType HeuristicValues::CalculateInflatedManaCostGivenToEnemy(const CardType & type)
{
    double cost = 0;

    for (const auto & createDescription : type.getBuyScript().getEffect().getCreate())
    {
        if (!createDescription.getOwn())
        {
            cost += GetInflatedManaCostValue(createDescription.getType()) * createDescription.getMultiple();
        }
    }
    
    return cost;
}

EvaluationType Heuristics::CurrentCardValue(const Card & blocker, const GameState & state)
{
    return DamageLoss_WillCost(blocker, state, blocker.currentHealth());
}

EvaluationType Heuristics::DamageLoss_WillCost(const Card & card, const GameState & state, const HealthType & damage)
{
    // if it's a doom card with 1 lifespan it's useless to us anyway
    if (card.getCurrentLifespan() == 1 || damage == 0)
    {
        return 0;
    }
    
    // add small epsilon loss for lifespan, charges and exhaust as tie breakers
    double epsilon = 0.001;
    double chargeLoss = card.getType().usesCharges() ? (1.0 / (1+card.getCurrentCharges())) * epsilon : 0;
    double lifespanLoss = card.getCurrentLifespan() > 0 ? (1.0 / card.getCurrentLifespan()) * epsilon : 0;
    double exhaustLoss = card.getCurrentDelay() > 0 ? (card.getCurrentDelay() * epsilon) : 0;
    double tieBreakLoss = chargeLoss + lifespanLoss + exhaustLoss;
    
    const CardType & type = card.getType();
    bool linearHealthValue = card.canBlockOnly() || card.getType().isAbilityHealthUserOnly();
    
    // an added value of attack if this card is currently resonating
    double resonateAttackAddedValue = AITools::GetReceiveFromResonators(card.getType(), state, card.getPlayer(), 1).amountOf(Resources::Attack) * WILL_VALUE_ATTACK
                                    + AITools::GetReceiveFromResonatees(card.getType(), state, card.getPlayer(), 1).amountOf(Resources::Attack) * WILL_VALUE_ATTACK;

    // special case of a 1hp blocker only, should be less value than engineer
    if (card.canBlockOnly() && card.currentHealth() == 1)
    {
        return 1.875 + resonateAttackAddedValue;
    }

    // use the custom heuristic values if they have been input by card designers
    double cardHeuristicValueMana  = type.hasCustomHeuristicValue() ? type.getCustomHeuristicValue() : HeuristicValues::Instance().GetInflatedManaCostValue(type);
    double cardHeuristicValueTotal = type.hasCustomHeuristicValue() ? type.getCustomHeuristicValue() : HeuristicValues::Instance().GetInflatedTotalCostValue(type);

    // if the card is fragile
    if (card.getType().isFragile())
    {
        HealthType damageTaken = damage < card.currentHealth() ? damage : card.currentHealth();
        double damageTakenRatio = ((double)damageTaken / type.getStartingHealth());

        // if the card can block only, the loss is proportional to its cost, since that's its only function
        if (linearHealthValue)
        {
            double cardValue = cardHeuristicValueMana - tieBreakLoss;
            double damageValue = damageTakenRatio * cardValue;

            // if the card isn't going to die, subtract an epsilon so we slightly favor having more units alive
            if (damageTaken < card.currentHealth())
            {
                damageValue -= epsilon;
            }

            // losing a single hp card should be worse than losing 1hp of a total card
            if (card.currentHealth() == 1)
            {
                damageValue += 2*epsilon;
            }

            return damageTaken >= card.currentHealth() ? (damageValue + resonateAttackAddedValue) : (damageValue);
        }
        // if the card has other functions, then there is an epsilon small loss from taking non-lethal damage
        else
        {
            double cardValue = cardHeuristicValueTotal - tieBreakLoss;

            cardValue -= tieBreakLoss;
            double damageValue = damageTakenRatio * cardValue;

            // if the card will die the loss is its full value
            // if not we give it an epsilon weighted value loss, just so it will choose to absorb if that option is available
            return damageTaken >= card.currentHealth() ? (cardValue + resonateAttackAddedValue) : (damageValue * epsilon);
        }
    }
    else 
    {
        // if a non-fragile card dies the loss is equal to its heuristic value
        if (damage >= card.currentHealth())
        {
            return (linearHealthValue ? cardHeuristicValueMana : cardHeuristicValueTotal) - tieBreakLoss + resonateAttackAddedValue;
        }
        // if it's non fragile and it doesn't die, there's no loss
        else
        {
            return 0;
        }
    }
}

EvaluationType Heuristics::DamageLoss_AttackValue(const Card & card, const GameState & state, const HealthType & damage)
{
    // if it's a doom card with 1 lifespan it's useless to us anyway
    if (card.getCurrentLifespan() == 1)
    {
        return 0;
    }

    // if a non-fragile card dies the loss is equal to its heuristic value
    if (damage >= card.currentHealth())
    {
        // if it produces chill, add this to the attack value
        HealthType chillAmount = (card.getType().getTargetAbilityType() == ActionTypes::CHILL) ? card.getType().getTargetAbilityAmount() : 0;

        return chillAmount + Heuristics::GetAttackProduced(card, state, card.getPlayer());
    }
    // if it's non fragile and it doesn't die, there's no loss
    else
    {
        return 0;
    }
    
}

EvaluationType Heuristics::BuyHighestCost(const CardType & type, const GameState & state, const PlayerID & player)
{
    return HeuristicValues::Instance().GetInflatedTotalCostValue(type);
}


EvaluationType Heuristics::SnipeHighestDefense(const Card & card, const GameState & state)
{
    if (card.canBlock())
    {
        return card.currentHealth();
    }

    return 0;
}

EvaluationType Heuristics::DefenseHeuristicSaveAttackers(const Card & card, const GameState & state)
{
    PRISMATA_ASSERT(!card.isDead(), "We are evaluating a dead card");

    EvaluationType attackProduced = card.getType().getAttack();

    if (card.getType().usesCharges() && card.getCurrentCharges() == 0)
    {
        attackProduced = 0;
    }

    PRISMATA_ASSERT(card.currentHealth() > 0, "Card health can't be zero here");

    return attackProduced / card.currentHealth();
}

HealthType Heuristics::GetAttackProduced(const CardType & type, const Script & script, const GameState & state, const PlayerID & player)
{
    HealthType attack = 0;
    attack += script.getEffect().getReceive().amountOf(Resources::Attack);
    if (script.hasResonate())
    {
        attack += AITools::GetReceiveFromResonatees(type, state, player, 1).amountOf(Resources::Attack);
        attack += AITools::GetReceiveFromResonators(type, state, player, 1).amountOf(Resources::Attack);
    }

    return attack;
}

HealthType Heuristics::GetAttackProduced(const CardType & type, const GameState & state, const PlayerID & player)
{
    return GetAttackProduced(type, type.getBeginOwnTurnScript(), state, player) + GetAttackProduced(type, type.getAbilityScript(), state, player);
}

HealthType Heuristics::GetAttackProduced(const Card & card, const GameState & state, const PlayerID & player)
{
    if (card.getCurrentLifespan() == 1)
    {
        return 0;
    }
    
    HealthType attack = 0;

    // if the card isn't out of charges add the ability script attack
    if (!card.getType().usesCharges() || card.getCurrentCharges() > 0)
    {
        attack += GetAttackProduced(card.getType(), card.getType().getAbilityScript(), state, player);
    }

    // add the begin turn attack
    attack += GetAttackProduced(card.getType(), card.getType().getBeginOwnTurnScript(), state, player);

    return attack;
}

EvaluationType Heuristics::BuyAttackValue(const CardType & type, const GameState & state, const PlayerID & player)
{
    EvaluationType attack = GetAttackProduced(type, state, player);
    EvaluationType val    = (attack / HeuristicValues::Instance().GetBuyTotalCost(type));

    bool singleUse = type.getAbilityScript().isSelfSac() || type.getBeginOwnTurnScript().isSelfSac();

    // we will allow resonate cards even if the buy limit has been exceeded
    bool resonates = AITools::NumResonatorsReady(type, state, player, 1) > 0 || AITools::NumResonateesReady(type, state, player, 1) > 0;

    const EvaluationType permanentBonus = 10000;
    const EvaluationType dominionBonus =  1000;

    if (val > 0)
    {
        // make sure attackers are bought first
        val += 100;

        // value permanent attackers over non-permanent
        val += (singleUse && !resonates) ? 0 : permanentBonus;
        
        // value dominion/resonate attackers over non-dominion
        val += (!type.isBaseSet() || resonates) ? dominionBonus : 0;
        
        return val;
    }
    else
    {
        return BuyHighestCost(type, state, player);
    }
    
}

EvaluationType Heuristics::BuyBlockValue(const CardType & type, const GameState & state, const PlayerID & player)
{
    EvaluationType block = type.canBlock(false) ? type.getStartingHealth() : 0;

    return (block*block) / HeuristicValues::Instance().GetBuyTotalCost(type);
}

bool Heuristics::CardActivateOrderComparator::operator() (CardID c1, CardID c2) const
{
    const Card & card1 = m_state.getCardByID(c1);
    const Card & card2 = m_state.getCardByID(c2);
    const CardType & t1 = card1.getType();
    const CardType & t2 = card2.getType();
    
    const int c1props[3] = { -static_cast<int>(t1.getID()),                                                           // 1. order by lower card type first
                                t1.getHealthUsed() > 0 ? card1.currentHealth() : -card1.currentHealth(),    // 2. if it uses health pick the healthiest, otherwise use the lowest one        
                                t1.usesCharges() ? card1.getCurrentCharges() : -10000, };                   // 3. pick the highest charged unit first

    const int c2props[3] = { -static_cast<int>(t2.getID()),                                                           // 1. order by lower card type first
                                t2.getHealthUsed() > 0 ? card2.currentHealth() : -card2.currentHealth(),    // 2. if it uses health pick the healthiest, otherwise use the lowest one        
                                t2.usesCharges() ? card2.getCurrentCharges() : -10000, };                   // 3. pick the highest charged unit first

    for (size_t i(0); i < 3; ++i)
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
