#include "CardType.h"
#include "CardTypeData.h"
#include "SacDescription.h"

using namespace Prismata;

const CardType CardTypes::None = CardType();

CardType::CardType()
    : _id(0)
{

}

CardType::CardType(const CardID id)
    : _id(id)
{

}

CardType::CardType(const CardType & type)
    : _id(type.getID())
{
 
}

CardType & CardType::operator = (const CardType & rhs)
{
    if (this != &rhs)
    {
        new (this) CardType(rhs);
    }

    return *this;
}

const CardID CardType::getID() const
{
    return _id;
}

const ActionID CardType::getActionType() const
{
    if (hasTargetAbility())
    {
        return getTargetAbilityType();
    }

    if (hasAbility())
    {
        return ActionTypes::USE_ABILITY;
    }

    return ActionTypes::NONE;
}

bool CardType::operator == (const CardType & rhs) const
{
    return getID() == rhs.getID();
}

bool CardType::operator != (const CardType & rhs) const
{
    return getID() != rhs.getID();
}

bool CardType::operator < (const CardType & rhs) const
{
    return getID() < rhs.getID();
}

bool CardType::hasAbility() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).hasAbility;  
}

bool CardType::usesCharges() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).startingCharge > 0;
}

bool CardType::isFragile() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).fragile;
}

bool CardType::isPromptBlocker() const
{
    return canBlock(false) && (getConstructionTime() == 0);
}

bool CardType::isFrontline() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).frontline;
}

bool CardType::canProduce(int m) const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).produces.amountOf(m) > 0;
}

const std::string & CardType::getName() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).cardName;
}

const std::string & CardType::getUIName() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).uiName;
}

const int & CardType::getCustomHeuristicValue() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).customHeuristicValue;
}

const HealthType & CardType::getAttack() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).attack;
}

const HealthType & CardType::getStartingHealth() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).startingHealth;
}

const HealthType & CardType::getHealthGained() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).healthGained;
}

bool CardType::isAbilityHealthUserOnly() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).isAbilityHealthUserOnly;
}

const HealthType & CardType::getHealthUsed() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).healthUsed;
}

const HealthType & CardType::getHealthMax() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).healthMax;
}

const std::vector<CardID> & CardType::getResonateFromIDs() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).resonatesFromIDs;
}

const std::vector<CardID> & CardType::getResonateToIDs() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).resonatesToIDs;
}

const SupplyType & CardType::getSupply() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).supply;
}

const Resources & CardType::getBuyCost() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).buyCost;
}

const HealthType & CardType::getBeginTurnAttackAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).beginTurnAttackAmount;
}

const HealthType & CardType::getAbilityAttackAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).abilityAttackAmount;
}

//
//const Mana & CardType::getAbilityCost() const
//{
//    return CardTypeData::Instance().getCardTypeInfo(_id).abilityCost;
//}

const TurnType & CardType::getLifespan() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).lifespan;
}

const TurnType & CardType::getConstructionTime() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).buildTime;
}

bool CardType::getDefaultBlocking() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).defaultBlocking;
}

bool CardType::getAssignedBlocking() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).assignedBlocking;
}

bool CardType::hasCustomHeuristicValue() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).hasCustomHeuristicValue;
}

const Script & CardType::getAbilityScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).abilityScript;
}

const Script & CardType::getBeginOwnTurnScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).beginOwnTurnScript;
}

const Script & CardType::getBuyScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).buyScript;
}

const Resources & CardType::produces() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).produces;
}

bool CardType::hasBeginOwnTurnScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).hasBeginOwnTurnScript;
}

const ChargeType & CardType::getStartingCharge() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).startingCharge;
}

bool CardType::usesBuySac() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).buySac.size() > 0;
}

bool CardType::isTech() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).isTech;
}

bool CardType::isSpell() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).isSpell;
}

const Condition & CardType::getTargetAbilityCondition() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).targetAbilityCondition;
}

const std::vector<SacDescription> & CardType::getBuySac() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).buySac;
}

const CardID CardType::getTypeBuySacCost(const CardType & type) const
{
    CardID sacced = 0;

    for (size_t i(0); i < getBuySac().size(); ++i)
    {
        if (getBuySac()[i].getType() == type)
        {
            sacced += getBuySac()[i].getMultiple();
        }
    }

    return sacced;
}

const Resources CardType::getCreatedUnitsManaProduced() const
{
    Resources produced;

    const std::vector<CreateDescription> & created = getBuyScript().getEffect().getCreate();
    for (size_t j(0); j < created.size(); ++j)
    {
        if (created[j].getOwn())
        {
            for (size_t k(0); k < created[j].getMultiple(); ++k)
            {
                produced.add(created[j].getType().getBeginOwnTurnScript().getEffect().getReceive());
            }
        }
    }

    return produced;
}

const std::vector<SacDescription> & CardType::getAbilitySac() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).abilitySac;
}

const ChargeType CardType::getChargeUsed() const
{
    return 1;
}

bool CardType::isEconCard() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).isEconCard;
}

bool CardType::isBaseSet() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).isBaseSet;
}

const HealthType & CardType::getAttackGivenToEnemy() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).attackGivenToEnemy;
}

const ActionID & CardType::getTargetAbilityType() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).targetActionType; 
}

const HealthType & CardType::getTargetAbilityAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).targetAmount;
}
    
bool CardType::hasTargetAbility() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).targetActionType != ActionTypes::NONE;
}

bool CardType::canBlock(bool assigned) const
{
    if (assigned)
    {
        return getAssignedBlocking();
    }
    else
    {
        return getDefaultBlocking();
    }
}

const std::string & CardType::getDescription() const
{
    return CardTypeData::Instance().getCardTypeInfo(_id).description;
}

const std::string CardType::getImageFileName() const
{
    std::stringstream ss;
    ss << getUIName() << ".png";
    return ss.str();
}
