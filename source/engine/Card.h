#pragma once

#include "Common.h"
#include "CardType.h"
#include "rapidjson/rapidjson.h"

namespace Prismata
{
enum CardRoles { };

namespace CardCreationMethod
{
enum { Bought, BuyScript, AbilityScript, Manual };
}

namespace AliveStatus
{
enum { Alive, Dead, KilledThisTurn };
}

namespace CauseOfDeath
{
enum { None, SelfSac, SelfAbilityHealthCost, Sniped, BuySacCost, AbilitySacCost, Blocker, Breached, Lifespan, UndoCreate, Unknown, Deleted, NumCausesOfDeath };
}

namespace DamageSource
{
enum { Block, Breach, NumDamageSources };
}

class Card
{
    CardType    m_type;
    CardID      m_id                 = (CardID)-1;
    CardID      m_targetID           = 0;
    PlayerID    m_player             = Players::Player_None;
    HealthType  m_currentHealth      = 0;
    HealthType  m_currentChill       = 0;
    HealthType  m_damageTaken        = 0;
    TurnType    m_constructionTime   = 0;
    TurnType    m_currentDelay       = 0;
    TurnType    m_lifespan           = 0;
    ChargeType  m_currentCharges     = 0;
    int         m_status             = CardStatus::Default;
    int         m_aliveStatus        = AliveStatus::Alive;
    int         m_causeOfDeath       = CauseOfDeath::None;
    bool        m_hasTarget          = false;
    bool        m_dead               = true;
    bool        m_sellable           = false;
    bool        m_inPlay             = false;
    bool        m_wasBreached        = false;
    bool        m_abilityUsedThisTurn = false;

    std::vector<CardID> m_killedCardIDs;
    std::vector<CardID> m_createdCardIDs;

public:

    Card();
    Card(const std::string & jsonString);
    Card(const rapidjson::Value & cardValue);
    Card(const CardType type, const PlayerID player, const int & creationMethod, const TurnType delay, const TurnType lifespan);
    Card & operator = (const Card & rhs);
    bool          operator == (const Card & rhs) const;
    bool          operator < (const Card & rhs) const;

    const CardType    getType() const;
    CardID      getID() const;
    CardID      getTargetID() const;
    PlayerID    getPlayer() const;
    HealthType  currentHealth() const;
    HealthType  currentChill() const;
    HealthType  getDamageTaken() const;
    int         getAliveStatus() const;
    ChargeType  getCurrentCharges() const;
    TurnType    getConstructionTime() const;
    TurnType    getCurrentLifespan() const;
    TurnType    getCurrentDelay() const;
    int         getStatus() const;

    bool canBlock() const;
    bool isUnderConstruction() const;
    bool isDead() const;
    bool canUseAbility() const;
    bool canUndoUseAbility() const;
    bool canRunBeginOwnTurnScript() const;
    bool canSac() const;
    bool isBreachable() const;
    bool isDelayed() const;
    bool meetsCondition(const Condition & condition) const;
    bool isIsomorphic(const Card & other) const;
    bool canBreachFor(const HealthType damage) const;
    bool isOverkillable() const;
    bool canOverkillFor(const HealthType damage) const;
    bool canBeChilled() const;
    bool canFrontlineFor(const HealthType damagee) const;
    bool canBlockOnly() const;
    bool isSellable() const;
    bool isInPlay() const;
    bool isFrozen() const;
    bool hasTarget() const;
    bool selfKilled() const;
    bool wasBreached() const;

    void setStatus(int status);
    void takeDamage(const HealthType amount, const int damageSource);
    void useAbility();
    void undoUseAbility();
    void runAbilityScript();
    void runBeginTurnScript();
    void beginTurn();
    void kill(const int causeOfDeath);
    void applyChill(const HealthType amount);
    void removeChill(const HealthType amount);
    void setID(const CardID id);
    void addKilledCardID(const CardID id);
    void addCreatedCardID(const CardID id);
    void undoKill();
    void endTurn();
    void setTargetID(const CardID targetID);
    void setInPlay(bool inPlay);
    void clearTarget();
    void undoBreach();

    const std::vector<CardID> & getKilledCardIDs() const;
    const std::vector<CardID> & getCreatedCardIDs() const;

    const std::string toJSONString(bool formatted = false) const;
};

class IsomorphicCardComparator
{
    const Card & _card;

public:

    IsomorphicCardComparator(const Card & card)
        : _card(card)
    {
    }

    bool operator() (const Card & c) const
    {
        return c.isIsomorphic(_card);
    }
};
}