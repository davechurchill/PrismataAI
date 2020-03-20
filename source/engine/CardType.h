#pragma once

#include "Common.h"
#include "Script.h"
#include "Resources.h"

namespace Prismata
{
 
namespace CardStatus
{
    enum {Default, Assigned, Inert, NUM_STATUS};
}

class SacDescription;
class Condition;
class CardType
{
    const CardID _id;

public:
 
    CardType();
    CardType(const CardID & id);
    CardType(const CardType & type);

    CardType & operator = (const CardType & rhs);

    const std::vector<SacDescription> & getBuySac()         const;
    const std::vector<SacDescription> & getAbilitySac()     const;
    const std::vector<CardID> & getResonateFromIDs()        const;
    const std::vector<CardID> & getResonateToIDs()          const;

    const CardID &              getID()                     const;
    const HealthType &          getAttack()                 const;
    const HealthType &          getAttackGivenToEnemy()     const;
    const HealthType &          getStartingHealth()         const;
    const HealthType &          getHealthGained()           const;
    const HealthType &          getHealthUsed()             const;
    const HealthType &          getHealthMax()              const;
    const HealthType &          getAbilityAttackAmount()    const;
    const HealthType &          getBeginTurnAttackAmount()  const;
    const SupplyType &          getSupply()                 const;
    const TurnType &            getLifespan()               const;
    const TurnType &            getConstructionTime()       const;
    const Script &              getAbilityScript()          const;
    const Script &              getBeginOwnTurnScript()     const;
    const Script &              getBuyScript()              const;
    const ActionID &            getTargetAbilityType()      const;
    const HealthType &          getTargetAbilityAmount()    const;
    const ActionID              getActionType()             const;
    const CardID                getTypeBuySacCost(const CardType & type) const;
    const int &                 getCustomHeuristicValue()   const;
    
    bool getDefaultBlocking()        const;
    bool getAssignedBlocking()       const;
    bool hasCustomHeuristicValue()   const;
    bool hasTargetAbility()          const;
    bool hasAbility()                const;
    bool hasBeginOwnTurnScript()     const;
    bool usesCharges()               const;
    bool usesBuySac()                const;
    bool canBlock(bool assigned)     const;
    bool canProduce(EnumType m)      const;
    bool isSpell()                   const;
    bool isTech()                    const;
    bool isFragile()                 const;
    bool isFrontline()               const;
    bool isPromptBlocker()           const;
    bool isEconCard()                const;
    bool isBaseSet()                 const;
    bool isAbilityHealthUserOnly()   const;

    const ChargeType &          getStartingCharge()         const;
    const ChargeType            getChargeUsed()             const;

    const Condition &           getTargetAbilityCondition() const;

    const Resources &                getBuyCost()                const;
    //const Mana &                getAbilityCost()            const;
    const Resources &                produces()                  const;
    const Resources                  getCreatedUnitsManaProduced()const;

    const std::string &         getName()                   const;
    const std::string &         getUIName()                 const;
    const std::string           getImageFileName()          const;
    const std::string &         getDescription()            const;

    bool operator == (const CardType & rhs)           const;
    bool operator != (const CardType & rhs)           const;
    bool operator <  (const CardType & rhs)           const;
};

namespace CardTypes
{
    void ResetData();
    void Init();
    const std::vector<CardType> & GetAllCardTypes();
    const std::vector<CardType> & GetBaseSetCardTypes();
    const std::vector<CardType> & GetDominionCardTypes();
    const CardType & GetCardType(const std::string & name);
    bool CardTypeExists(const std::string & name);
    bool IsBaseSet(const CardType & type);
    extern const CardType None;
}
}