#include "Card.h"

using namespace Prismata;

Card::Card()
{

}

Card::Card(const std::string & jsonString)
{
    rapidjson::Document document;
    bool parsingFailed = document.Parse(jsonString.c_str()).HasParseError();

    PRISMATA_ASSERT(!parsingFailed, "Couldn't parse Card string JSON");

    (*this) = Card(document);
}

Card::Card(const rapidjson::Value & cardValue)
{
    m_inPlay = true;
    m_dead = false;

    // grab the card type and make sure it's a string and set the type
    PRISMATA_ASSERT(cardValue.HasMember("cardName"), "cardValue has no cardName property");
    const rapidjson::Value & cardNameVal = cardValue["cardName"];
    PRISMATA_ASSERT(cardNameVal.IsString(), "cardNameVal is not a string");
    m_type = CardTypes::GetCardType(cardNameVal.GetString());

    // set the base default values for this card in case they are not included in the description
    m_currentHealth = m_type.getStartingHealth();
    m_currentCharges = m_type.getStartingCharge();
    m_lifespan = m_type.getLifespan();

    for (rapidjson::Value::ConstMemberIterator itr = cardValue.MemberBegin(); itr != cardValue.MemberEnd(); ++itr)
    {
        const std::string &         prop = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;

        if (prop == "hp" || prop == "health")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON hp was not an Int");
            m_currentHealth = val.GetInt();
        }
        else if (prop == "role")
        {
            PRISMATA_ASSERT(val.IsString(), "GameState JSON role was not a String");

            const std::string & role = val.GetString();
            if (role == "default")
            {
                m_status = CardStatus::Default;
            }
            else if (role == "assigned")
            {
                m_status = CardStatus::Assigned;
            }
            else if (role == "inert")
            {
                m_status = CardStatus::Inert;
            }
            else if (role == "sellable")
            {
                m_status = CardStatus::Inert;
                m_sellable = true;
            }
        }
        else if (prop == "color" || prop == "owner")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON color was not an Int");
            m_player = val.GetInt();
        }
        else if (prop == "disrupt" || prop == "disruptDamage")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON disrupt was not an Int");
            m_currentChill = val.GetInt();
        }
        else if (prop == "blocking")
        {
            PRISMATA_ASSERT(val.IsBool(), "GameState JSON blocking was not a Bool");
        }
        else if (prop == "lifespan")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON lifespan was not an Int");
            int lifespanVal = val.GetInt();
            m_lifespan = lifespanVal == -1 ? 0 : val.GetInt();
        }
        else if (prop == "charge")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON charge was not an Int");
            m_currentCharges = val.GetInt();
        }
        else if (prop == "delay")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON delay was not an Int");
            m_currentDelay = val.GetInt();
        }
        else if (prop == "target")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON target was not an Int");
            m_targetID = val.GetInt();
        }
        else if (prop == "constructionTime")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON construction was not an Int");
            m_constructionTime = val.GetInt();
        }
        else if (prop == "dead")
        {
            PRISMATA_ASSERT(val.IsBool(), "GameState JSON dead was not a Bool");
            m_dead = val.GetBool();
        }
        else if (prop == "instId")
        {
            // do nothing, we use our own instID
        }
        else if (prop == "deadness") 
        { 
            PRISMATA_ASSERT(val.IsString(), "GameState JSON deadness was not a Bool");

            const std::string & dead = val.GetString();

            if (dead == "selfsacced")
            {
                m_dead = true;
                m_causeOfDeath = CauseOfDeath::SelfSac;
            }
            else if (dead == "sacced")
            {
                m_dead = true;
                m_causeOfDeath = CauseOfDeath::AbilitySacCost;
            }
            else if (dead == "alive")
            {
                m_dead = false;
            }
            else if (dead == "dead")
            {
                m_dead = true;
                m_causeOfDeath = CauseOfDeath::Unknown;
            }
            else if (dead == "blocked")
            {
                m_dead = true;
                m_causeOfDeath = CauseOfDeath::Blocker;
            }
            else if (dead == "breached")
            {
                m_dead = true;
                m_causeOfDeath = CauseOfDeath::Breached;
            }
            else
            {
                PRISMATA_ASSERT(false, "Unknown deadness value: %s", dead.c_str());
            }
        }
        else if (prop == "disruptorIds") { }
        else if (prop == "buyCreateIds") { }
        else if (prop == "creatorIdFromBeginTurn") { }
        else if (prop == "beginOwnTurnCreateIds") { }
        else if (prop == "abilityCreateIds") { }
        else if (prop == "buyCreateIds") { }
        else if (prop == "creatorIdFromBuyOrAbility") { }
        else if (prop == "sniperId") { }
        else if (prop == "laneId") { }
        else if (prop == "damage") { }
        else if (prop == "cardName") { }
        else if (prop == "amount") { }
        else
        {
            PRISMATA_ASSERT(false, "Unknown Card instance property: %s", prop.c_str());
        }
    }
}
   
Card::Card(const CardType type, const PlayerID player, const int & creationMethod, const TurnType delay, const TurnType lifespan)
    : m_type                 (type)
    , m_player               (player)
    , m_id                   (-1)
    , m_currentCharges       (type.getStartingCharge())
    , m_constructionTime     (0)
    , m_currentHealth        (type.getStartingHealth())
    , m_damageTaken          (0)
    , m_currentChill         (0)
    , m_lifespan             (lifespan == 0 ? type.getLifespan() : lifespan)
    , m_aliveStatus          (AliveStatus::Alive)
    , m_causeOfDeath         (CauseOfDeath::None)
    , m_status               (CardStatus::Inert)
    , m_currentDelay         (0)
    , m_targetID             (0)
    , m_hasTarget            (false)
    , m_abilityUsedThisTurn  (false)
    , m_dead                 (false)
    , m_sellable             (false)
    , m_inPlay               (true)
    , m_wasBreached          (false)
{
    switch (creationMethod)
    {
        case CardCreationMethod::Bought:
        {
            m_constructionTime = type.getConstructionTime();
            m_status = CardStatus::Inert;
            m_sellable = true;
            break;
        }
        case CardCreationMethod::AbilityScript:
        {
            m_status = CardStatus::Inert;
            m_currentDelay = delay;
            break;
        }
        case CardCreationMethod::BuyScript:
        {
            m_status = CardStatus::Inert;
            m_constructionTime = delay;
            break;
        }
        case CardCreationMethod::Manual:
        {
            if (type.hasAbility() || type.hasTargetAbility())
            {
                m_status = CardStatus::Default;
            }
                
            m_currentDelay = delay;
            break;
        }
        default:
        {
            PRISMATA_ASSERT(false, "Unknown card creation method");
            break;
        }
    }
}

void Card::setID(const CardID id)
{
    m_id = id;
}

TurnType Card::getConstructionTime() const
{
    return m_constructionTime;
}

TurnType Card::getCurrentLifespan() const
{
    return m_lifespan;
}

TurnType Card::getCurrentDelay() const
{
    return m_currentDelay;
}

Card & Card::operator = (const Card & rhs)
{
    if (this != &rhs)
    {
        new (this) Card(rhs);
    }

    return *this;
}


const CardType Card::getType() const
{
 return m_type;
}

PlayerID Card::getPlayer() const
{
    return m_player;
}

CardID Card::getID() const
{
 return m_id;
}

bool Card::isInPlay() const
{
    return m_inPlay;
}

bool Card::isFrozen() const
{
    return currentChill() >= currentHealth();
}

void Card::setInPlay(bool inPlay)
{
    m_inPlay = inPlay;
}

bool Card::hasTarget() const
{
    return m_hasTarget;
}

bool Card::operator == (const Card & rhs) const
{
    return getID() == rhs.getID();
}

bool Card::operator < (const Card & rhs) const
{
    return getID() < rhs.getID();
}

bool Card::isDead() const
{
    return m_dead;
}

HealthType Card::currentHealth() const
{
    return m_currentHealth;
}

HealthType Card::currentChill() const
{
    return m_currentChill;
}

int Card::getStatus() const
{
    return m_status;
}

int Card::getAliveStatus() const
{
    return m_aliveStatus;
}

bool Card::isBreachable() const
{
    return !isUnderConstruction() && !isDead();
}

bool Card::isOverkillable() const
{
    return isUnderConstruction() && !isDead();
}

bool Card::canBreachFor(const HealthType damage) const
{
    if (damage == 0)
    {
        return false;
    }

    if (!isBreachable())
    {
        return false;
    }

    if (!getType().isFragile() && damage < currentHealth())
    {
        return false;
    }

    return true;
}

bool Card::canOverkillFor(const HealthType damage) const
{
    if (damage == 0)
    {
        return false;
    }

    if (!isOverkillable())
    {
        return false;
    }

    if (!getType().isFragile() && damage < currentHealth())
    {
        return false;
    }

    return true;
}

void Card::takeDamage(const HealthType amount, const int damageSource)
{
    m_damageTaken = std::min(amount, m_currentHealth);

    if (amount >= m_currentHealth)
    {
        switch (damageSource)
        {
            case DamageSource::Block:
            {
                kill(CauseOfDeath::Blocker);
                break;
            }
            case DamageSource::Breach:
            {
                kill(CauseOfDeath::Breached);
                m_wasBreached = true;
                break;
            }
            default:
            {
                PRISMATA_ASSERT(false, "Unknown damage source type: %d", damageSource);
            }
        }
    }

    if (getType().isFragile())
    {
        m_currentHealth -= std::min(amount, m_currentHealth);
        if (damageSource == DamageSource::Breach)
        {
            m_wasBreached = true;
        }
    }
}

void Card::applyChill(const HealthType amount)
{
    PRISMATA_ASSERT(currentChill() < currentHealth(), "We shouldn't be applying chill to a frozen card");

    m_currentChill += amount;
}

void Card::removeChill(const HealthType amount)
{
    PRISMATA_ASSERT(amount <= currentChill(), "Trying to remove too much chill from a card");

    m_currentChill -= amount;
}

void Card::setStatus(int status)
{
    m_status = status;
}

void Card::clearTarget()
{
    m_hasTarget = false;
    m_targetID = 0;
}

void Card::setTargetID(const CardID targetID)
{
    m_hasTarget = true;
    m_targetID = targetID;
}

CardID Card::getTargetID() const
{
    PRISMATA_ASSERT(hasTarget(), "Trying to get the target of a card without one: %s", getType().getUIName().c_str() );

    return m_targetID;
}

bool Card::canSac() const
{
    if (getType().hasAbility() && getStatus() != CardStatus::Assigned)
    {
        return false;
    }

    if (isDelayed() || isUnderConstruction() || isDead())
    {
        return false;
    }

    return true;
}

bool Card::canBlock() const
{
    if (!getType().canBlock(getStatus() == CardStatus::Assigned))
    {
        return false;
    }

    if (getCurrentDelay() > 0)
    {
        return false;
    }

    if (isUnderConstruction())
    {
        return false;
    }

    if (isDead())
    {
        return false;
    }

    if (isFrozen())
    {
        return false;
    }
    
    return true;
}

void Card::kill(const int causeOfDeath)
{
    m_dead = true;
    m_aliveStatus = AliveStatus::KilledThisTurn;
    m_causeOfDeath = causeOfDeath;
}

bool Card::canBeChilled() const
{
    if (!canBlock())
    {
        return false;
    }

    if (currentChill() >= currentHealth())
    {
        return false;
    }

    if (isUnderConstruction())
    {
        return false;
    }

    return true;
}

void Card::undoBreach()
{
    PRISMATA_ASSERT(m_wasBreached, "Can't un-breach a card that wasn't breached");

    if (getType().isFragile())
    {
        m_currentHealth += m_damageTaken;
    }
    else
    {
        m_currentHealth = getType().getStartingHealth();
    }

    
    m_wasBreached = false;
    m_damageTaken = 0;
}

bool Card::isUnderConstruction() const
{
    return getConstructionTime() > 0;
}

bool Card::isDelayed() const
{
    return m_currentDelay > 0;
}

ChargeType Card::getCurrentCharges() const
{
    return m_currentCharges;
}

void Card::beginTurn()
{
    // card is no longer sellable
    m_sellable = false;
    m_damageTaken = 0;
    m_wasBreached = false;
    m_abilityUsedThisTurn = false;
    m_killedCardIDs.clear();
    m_createdCardIDs.clear();
    clearTarget();

    // update the alive status
    if (m_aliveStatus == AliveStatus::KilledThisTurn)
    {
        m_aliveStatus = AliveStatus::Dead;
        return;
    }

    // reduce lifespan
    if (!isUnderConstruction() && !isDelayed() && m_lifespan > 0)
    {
        --m_lifespan;
        
        if (m_lifespan == 0)
        {
            kill(CauseOfDeath::Lifespan);
            return;
        }
    }

    // reduce delay
    if (!isUnderConstruction() && isDelayed())
    {
        --m_currentDelay;
    }

    if (isDelayed())
    {
        setStatus(CardStatus::Inert);
    }

    // reduce construction time
    if (isUnderConstruction())
    {
        m_constructionTime--;
    }
    
    // do everything else post-construction
    if (!isUnderConstruction() && !isDelayed())
    {
        // gain healthgained
        m_currentHealth += m_type.getHealthGained();
        if (m_type.getHealthMax() > 0 && m_currentHealth > m_type.getHealthMax())
        {
            m_currentHealth = m_type.getHealthMax();
        }

        // set default status
        if (getType().hasAbility() || getType().hasTargetAbility())
        {
            setStatus(CardStatus::Default);
        }
		else
		{
			setStatus(CardStatus::Inert);
		}

        m_currentChill = 0;
    }
}

bool Card::canRunBeginOwnTurnScript() const
{
    return !isUnderConstruction() && m_currentDelay == 0;
}

bool Card::canFrontlineFor(const HealthType damage) const
{
	if (!getType().isFrontline())
	{
		return false;
	}

	if (isUnderConstruction())
	{
		return false;
	}

	if (!getType().isFragile() && (damage < currentHealth()))
	{
		return false;
	}

	return true;
}

bool Card::canUseAbility() const
{
    if (isDead())
    {
        return false;
    }

    if (getStatus() != CardStatus::Default)
    {
        return false;
    }

    if (isUnderConstruction())
    {
        return false;
    }

    if (getType().usesCharges() && (getCurrentCharges() < getType().getChargeUsed()))
    {
        return false;
    }

    if (m_currentDelay > 0)
    {
        return false;
    }

    if (m_currentHealth < m_type.getHealthUsed())
    {
        return false;
    }

    return true;
}

void Card::runAbilityScript()
{
    m_currentDelay = m_type.getAbilityScript().getDelay();

    if (m_type.getAbilityScript().isSelfSac())
    {
        kill(CauseOfDeath::SelfSac);
    }
}

void Card::runBeginTurnScript()
{
    PRISMATA_ASSERT(canRunBeginOwnTurnScript(), "runBeginTurnScript() called when canRunBeginOwnTurnScript() is false");

    m_currentDelay = m_type.getBeginOwnTurnScript().getDelay();
}

bool Card::canUndoUseAbility() const
{
    if (m_status != CardStatus::Assigned)
    {
        return false;
    }

    if (m_dead && (m_aliveStatus != AliveStatus::KilledThisTurn))
    {
        return false;
    }

    return true;
}

void Card::undoUseAbility()
{
    PRISMATA_ASSERT(canUndoUseAbility(), "Trying to undo an ability that we can't");

    if (getType().usesCharges())
    {
        m_currentCharges += getType().getChargeUsed();
    }

    if (m_type.getHealthUsed() > 0 && m_currentHealth == 0)
    {
        m_aliveStatus = AliveStatus::Alive;
        m_causeOfDeath = CauseOfDeath::None;
    }

    m_currentHealth += m_type.getHealthUsed();

    if (m_type.getAbilityScript().isSelfSac())
    {
        m_aliveStatus = AliveStatus::Alive;
        m_causeOfDeath = CauseOfDeath::None;
    }

    setStatus(CardStatus::Default);
    m_abilityUsedThisTurn = false;
    m_currentDelay = 0;
    m_killedCardIDs.clear();
}

void Card::endTurn()
{
    m_killedCardIDs.clear();
    m_createdCardIDs.clear();
    clearTarget();

    PRISMATA_ASSERT(m_createdCardIDs.size() == 0, "WTF");
}

void Card::useAbility()
{
    if (!canUseAbility())
    {
        bool b = canUseAbility();
    }

    PRISMATA_ASSERT(canUseAbility(), "useAbility() called when canUseAbility() is false: %s", getType().getName().c_str());

    m_abilityUsedThisTurn = true;

    if (getType().usesCharges())
    {
        m_currentCharges -= getType().getChargeUsed();
    }
        
    m_currentHealth -= m_type.getHealthUsed();

    if (m_currentHealth == 0)
    {
        kill(CauseOfDeath::SelfAbilityHealthCost);
    }
               
    setStatus(CardStatus::Assigned);

    runAbilityScript();
}

bool Card::meetsCondition(const Condition & condition) const
{
    if (isUnderConstruction())
    {
        return false;
    }

    if (condition.hasCardType() && condition.getTypeID() != 0 && condition.getTypeID() != m_type.getID())
    {
        return false;
    }

    if (condition.isNotBlocking() && canBlock())
    {
        return false;
    }

    if (condition.isTech() && !m_type.isTech())
    {
        return false;
    }

    if (condition.hasHealthCondition() && (currentHealth() > condition.getHealthAtMost()))
    {
        return false;
    }

    return true;
}

bool Card::canBlockOnly() const
{
    if (!getType().canBlock(false))
    {
        return false;
    }

    if (getType().hasBeginOwnTurnScript())
    {
        return false;
    }

    if (getType().hasTargetAbility())
    {
        return false;
    }

    if (getType().hasAbility())
    {
        if (!getType().usesCharges())
        {
            return false;
        }
        else
        {
            if (getCurrentCharges() >= getType().getChargeUsed())
            {
                return false;
            }
        }
    }

    return true;
}

bool Card::isIsomorphic(const Card & other) const
{
    return          getType()               == other.getType() 
                &&  getPlayer()             == other.getPlayer()
                &&  currentHealth()         == other.currentHealth()
                &&  currentChill()          == other.currentChill()
                &&  getCurrentCharges()     == other.getCurrentCharges()
                &&  isDead()                == other.isDead()
                &&  getCurrentDelay()       == other.getCurrentDelay()
                &&  getConstructionTime()   == other.getConstructionTime()
                &&  getCurrentLifespan()    == other.getCurrentLifespan()
                &&  getStatus()             == other.getStatus();
}

void Card::addCreatedCardID(const CardID id)
{
    m_createdCardIDs.push_back(id);
}

void Card::addKilledCardID(const CardID id)
{
    m_killedCardIDs.push_back(id);
}

void Card::undoKill()
{
    PRISMATA_ASSERT(isDead(), "Can't undo kill on a non-dead card");

    m_aliveStatus = AliveStatus::Alive;
    m_dead = false;
    m_causeOfDeath = CauseOfDeath::None;
}

const std::vector<CardID> & Card::getKilledCardIDs() const
{
    return m_killedCardIDs;
}

HealthType Card::getDamageTaken() const
{
    return m_damageTaken;
}

bool Card::wasBreached() const
{
    return m_wasBreached;
}

bool Card::selfKilled() const
{
    return (m_causeOfDeath == CauseOfDeath::SelfSac) || (m_causeOfDeath == CauseOfDeath::SelfAbilityHealthCost);
}

bool Card::isSellable() const
{
    return m_sellable;
}

const std::vector<CardID> & Card::getCreatedCardIDs() const
{
    return m_createdCardIDs;
}

const std::string Card::toJSONString(bool formatted) const
{
    const char q = '\"';
    std::stringstream ss;

    ss << "{"; if (formatted) { ss << std::endl; }
    
    if (formatted) { ss << "    "; } ss << "\"owner\":"         << (int)getPlayer()     << ", ";    if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"cardName\":\""    << getType().getName()  << "\", ";  if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"health\":"        << (int)currentHealth() << ", ";    if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"disruptDamage\":" << (int)currentChill()  << ", ";    if (formatted) { ss << std::endl; }

    std::string deadString = "alive";

    if (isDead())
    {
        if (getStatus() == CardStatus::Assigned && getType().getAbilityScript().isSelfSac())
        {
            deadString = "selfsacced";
        }
        else
        {
            deadString = "dead";
        }
    }

    if (formatted) { ss << "    "; } ss << "\"deadness\":\"" << deadString << "\", "; if (formatted) { ss << std::endl; }

    if (m_sellable)
    {
        if (formatted) { ss << "    "; } ss << "\"role\":\"sellable\", "; if (formatted) { ss << std::endl; }
    }
    else
    {
        switch (getStatus())
        {
            case CardStatus::Default:
            {
                if (formatted) { ss << "    "; } ss << "\"role\":\"default\", "; if (formatted) { ss << std::endl; }
                break;
            }
            case CardStatus::Assigned:
            {
                if (formatted) { ss << "    "; } ss << "\"role\":\"assigned\", "; if (formatted) { ss << std::endl; }
                break;
            }
            case CardStatus::Inert:
            {
                if (formatted) { ss << "    "; } ss << "\"role\":\"inert\", "; if (formatted) { ss << std::endl; }
                break;
            }
        }
    }

    if (formatted) { ss << "    "; } ss << "\"constructionTime\":" << (int)getConstructionTime() << ", "; if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"charge\":" << (int)getCurrentCharges() << ", "; if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"delay\":" << (int)getCurrentDelay() << ", "; if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"lifespan\":" << (int)(getCurrentLifespan() == 0 ? -1 : getCurrentLifespan()) << ", "; if (formatted) { ss << std::endl; }
    if (formatted) { ss << "    "; } ss << "\"blocking\":" << (canBlock() ? "true" : "false"); if (formatted) { ss << std::endl; }

    ss << "}";

    return ss.str();
}