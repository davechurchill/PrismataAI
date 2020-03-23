#pragma once

#include "Common.h"
#include "Script.h"
#include "Resources.h"

namespace Prismata
{

namespace CardStatus
{
enum { Default, Assigned, Inert, NUM_STATUS };
}

class SacDescription;
class Condition;
class CardType
{
    const CardID m_id;

public:

    CardType();
    CardType(const CardID id);
    CardType(const CardType & type);

    CardType & operator = (const CardType rhs);

    const std::vector<SacDescription> & getBuySac()         const;
    const std::vector<SacDescription> & getAbilitySac()     const;
    const std::vector<CardID> & getResonateFromIDs()        const;
    const std::vector<CardID> & getResonateToIDs()          const;

    CardID          getID()                     const;
    HealthType      getAttack()                 const;
    HealthType      getAttackGivenToEnemy()     const;
    HealthType      getStartingHealth()         const;
    HealthType      getHealthGained()           const;
    HealthType      getHealthUsed()             const;
    HealthType      getHealthMax()              const;
    HealthType      getAbilityAttackAmount()    const;
    HealthType      getBeginTurnAttackAmount()  const;
    SupplyType      getSupply()                 const;
    TurnType        getLifespan()               const;
    TurnType        getConstructionTime()       const;
    const Script &  getAbilityScript()          const;
    const Script &  getBeginOwnTurnScript()     const;
    const Script &  getBuyScript()              const;
    ActionID        getTargetAbilityType()      const;
    HealthType      getTargetAbilityAmount()    const;
    ActionID        getActionType()             const;
    CardID          getTypeBuySacCost(const CardType type) const;
    int             getCustomHeuristicValue()   const;

    bool getDefaultBlocking()        const;
    bool getAssignedBlocking()       const;
    bool hasCustomHeuristicValue()   const;
    bool hasTargetAbility()          const;
    bool hasAbility()                const;
    bool hasBeginOwnTurnScript()     const;
    bool usesCharges()               const;
    bool usesBuySac()               const;
    bool canBlock(bool assigned)    const;
    bool canProduce(int m)          const;
    bool isSpell()                   const;
    bool isTech()                    const;
    bool isFragile()                 const;
    bool isFrontline()               const;
    bool isPromptBlocker()           const;
    bool isEconCard()                const;
    bool isBaseSet()                 const;
    bool isAbilityHealthUserOnly()   const;

    const ChargeType          getStartingCharge()         const;
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

    bool operator == (const CardType rhs)           const;
    bool operator != (const CardType rhs)           const;
    bool operator <  (const CardType rhs)           const;
};

namespace CardTypes
{
    void ResetData();
    void Init();
    const std::vector<CardType> & GetAllCardTypes();
    const std::vector<CardType> & GetBaseSetCardTypes();
    const std::vector<CardType> & GetDominionCardTypes();
    CardType GetCardType(const std::string & name);
    bool CardTypeExists(const std::string & name);
    bool IsBaseSet(const CardType type);
    extern const CardType None;
}
}