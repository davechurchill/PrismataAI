#include "CardType.h"
#include "CardTypeData.h"
#include "SacDescription.h"

using namespace Prismata;

const CardType CardTypes::None = CardType();

CardType::CardType()
    : m_id(0)
{

}

CardType::CardType(const CardID id)
    : m_id(id)
{

}

CardType::CardType(const CardType & type)
    : m_id(type.getID())
{
 
}

CardType & CardType::operator = (const CardType rhs)
{
    if (this != &rhs)
    {
        new (this) CardType(rhs);
    }

    return *this;
}

CardID CardType::getID() const
{
    return m_id;
}

ActionID CardType::getActionType() const
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

bool CardType::operator == (const CardType rhs) const
{
    return getID() == rhs.getID();
}

bool CardType::operator != (const CardType rhs) const
{
    return getID() != rhs.getID();
}

bool CardType::operator < (const CardType rhs) const
{
    return getID() < rhs.getID();
}

bool CardType::hasAbility() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).hasAbility;  
}

bool CardType::usesCharges() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).startingCharge > 0;
}

bool CardType::isFragile() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).fragile;
}

bool CardType::isPromptBlocker() const
{
    return canBlock(false) && (getConstructionTime() == 0);
}

bool CardType::isFrontline() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).frontline;
}

bool CardType::canProduce(int m) const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).produces.amountOf(m) > 0;
}

const std::string & CardType::getName() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).cardName;
}

const std::string & CardType::getUIName() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).uiName;
}

int CardType::getCustomHeuristicValue() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).customHeuristicValue;
}

HealthType CardType::getAttack() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).attack;
}

HealthType CardType::getStartingHealth() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).startingHealth;
}

HealthType CardType::getHealthGained() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).healthGained;
}

bool CardType::isAbilityHealthUserOnly() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).isAbilityHealthUserOnly;
}

HealthType CardType::getHealthUsed() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).healthUsed;
}

HealthType CardType::getHealthMax() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).healthMax;
}

const std::vector<CardID> & CardType::getResonateFromIDs() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).resonatesFromIDs;
}

const std::vector<CardID> & CardType::getResonateToIDs() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).resonatesToIDs;
}

SupplyType CardType::getSupply() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).supply;
}

const Resources & CardType::getBuyCost() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).buyCost;
}

HealthType CardType::getBeginTurnAttackAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).beginTurnAttackAmount;
}

HealthType CardType::getAbilityAttackAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).abilityAttackAmount;
}

//
//const Mana & CardType::getAbilityCost() const
//{
//    return CardTypeData::Instance().getCardTypeInfo(_id).abilityCost;
//}

TurnType CardType::getLifespan() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).lifespan;
}

TurnType CardType::getConstructionTime() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).buildTime;
}

bool CardType::getDefaultBlocking() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).defaultBlocking;
}

bool CardType::getAssignedBlocking() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).assignedBlocking;
}

bool CardType::hasCustomHeuristicValue() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).hasCustomHeuristicValue;
}

const Script & CardType::getAbilityScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).abilityScript;
}

const Script & CardType::getBeginOwnTurnScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).beginOwnTurnScript;
}

const Script & CardType::getBuyScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).buyScript;
}

const Resources & CardType::produces() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).produces;
}

bool CardType::hasBeginOwnTurnScript() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).hasBeginOwnTurnScript;
}

const ChargeType CardType::getStartingCharge() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).startingCharge;
}

bool CardType::usesBuySac() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).buySac.size() > 0;
}

bool CardType::isTech() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).isTech;
}

bool CardType::isSpell() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).isSpell;
}

const Condition & CardType::getTargetAbilityCondition() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).targetAbilityCondition;
}

const std::vector<SacDescription> & CardType::getBuySac() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).buySac;
}

CardID CardType::getTypeBuySacCost(const CardType type) const
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
    return CardTypeData::Instance().getCardTypeInfo(m_id).abilitySac;
}

const ChargeType CardType::getChargeUsed() const
{
    return 1;
}

bool CardType::isEconCard() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).isEconCard;
}

bool CardType::isBaseSet() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).isBaseSet;
}

HealthType CardType::getAttackGivenToEnemy() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).attackGivenToEnemy;
}

ActionID CardType::getTargetAbilityType() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).targetActionType; 
}

HealthType CardType::getTargetAbilityAmount() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).targetAmount;
}
    
bool CardType::hasTargetAbility() const
{
    return CardTypeData::Instance().getCardTypeInfo(m_id).targetActionType != ActionTypes::NONE;
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
    return CardTypeData::Instance().getCardTypeInfo(m_id).description;
}

const std::string CardType::getImageFileName() const
{
    std::stringstream ss;
    ss << getUIName() << ".png";
    return ss.str();
}
