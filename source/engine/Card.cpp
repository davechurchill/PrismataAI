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
    _inPlay = true;
    _dead = false;

    // grab the card type and make sure it's a string and set the type
    PRISMATA_ASSERT(cardValue.HasMember("cardName"), "cardValue has no cardName property");
    const rapidjson::Value & cardNameVal = cardValue["cardName"];
    PRISMATA_ASSERT(cardNameVal.IsString(), "cardNameVal is not a string");
    _type = CardTypes::GetCardType(cardNameVal.GetString());

    // set the base default values for this card in case they are not included in the description
    _currentHealth = _type.getStartingHealth();
    _currentCharges = _type.getStartingCharge();
    _lifespan = _type.getLifespan();

    for (rapidjson::Value::ConstMemberIterator itr = cardValue.MemberBegin(); itr != cardValue.MemberEnd(); ++itr)
    {
        const std::string &         prop = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;

        if (prop == "hp" || prop == "health")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON hp was not an Int");
            _currentHealth = val.GetInt();
        }
        else if (prop == "role")
        {
            PRISMATA_ASSERT(val.IsString(), "GameState JSON role was not a String");

            const std::string & role = val.GetString();
            if (role == "default")
            {
                _status = CardStatus::Default;
            }
            else if (role == "assigned")
            {
                _status = CardStatus::Assigned;
            }
            else if (role == "inert")
            {
                _status = CardStatus::Inert;
            }
            else if (role == "sellable")
            {
                _status = CardStatus::Inert;
                _sellable = true;
            }
        }
        else if (prop == "color" || prop == "owner")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON color was not an Int");
            _player = val.GetInt();
        }
        else if (prop == "disrupt" || prop == "disruptDamage")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON disrupt was not an Int");
            _currentChill = val.GetInt();
        }
        else if (prop == "blocking")
        {
            PRISMATA_ASSERT(val.IsBool(), "GameState JSON blocking was not a Bool");
        }
        else if (prop == "lifespan")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON lifespan was not an Int");
            int lifespanVal = val.GetInt();
            _lifespan = lifespanVal == -1 ? 0 : val.GetInt();
        }
        else if (prop == "charge")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON charge was not an Int");
            _currentCharges = val.GetInt();
        }
        else if (prop == "delay")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON delay was not an Int");
            _currentDelay = val.GetInt();
        }
        else if (prop == "target")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON target was not an Int");
            _targetID = val.GetInt();
        }
        else if (prop == "constructionTime")
        {
            PRISMATA_ASSERT(val.IsInt(), "GameState JSON construction was not an Int");
            _constructionTime = val.GetInt();
        }
        else if (prop == "dead")
        {
            PRISMATA_ASSERT(val.IsBool(), "GameState JSON dead was not a Bool");
            _dead = val.GetBool();
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
                _dead = true;
                _causeOfDeath = CauseOfDeath::SelfSac;
            }
            else if (dead == "sacced")
            {
                _dead = true;
                _causeOfDeath = CauseOfDeath::AbilitySacCost;
            }
            else if (dead == "alive")
            {
                _dead = false;
            }
            else if (dead == "dead")
            {
                _dead = true;
                _causeOfDeath = CauseOfDeath::Unknown;
            }
            else if (dead == "blocked")
            {
                _dead = true;
                _causeOfDeath = CauseOfDeath::Blocker;
            }
            else if (dead == "breached")
            {
                _dead = true;
                _causeOfDeath = CauseOfDeath::Breached;
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
   
Card::Card(const CardType & type, const PlayerID player, const int & creationMethod, const TurnType & delay, const TurnType & lifespan)
    : _type                 (type)
    , _player               (player)
    , _id                   (-1)
    , _currentCharges       (type.getStartingCharge())
    , _constructionTime     (0)
    , _currentHealth        (type.getStartingHealth())
    , _damageTaken          (0)
    , _currentChill         (0)
    , _lifespan             (lifespan == 0 ? type.getLifespan() : lifespan)
    , _aliveStatus          (AliveStatus::Alive)
    , _causeOfDeath         (CauseOfDeath::None)
    , _status               (CardStatus::Inert)
    , _currentDelay         (0)
    , _targetID             (0)
    , _hasTarget            (false)
    , _abilityUsedThisTurn  (false)
    , _dead                 (false)
    , _sellable             (false)
    , _inPlay               (true)
    , _wasBreached          (false)
{
    switch (creationMethod)
    {
        case CardCreationMethod::Bought:
        {
            _constructionTime = type.getConstructionTime();
            _status = CardStatus::Inert;
            _sellable = true;
            break;
        }
        case CardCreationMethod::AbilityScript:
        {
            _status = CardStatus::Inert;
            _currentDelay = delay;
            break;
        }
        case CardCreationMethod::BuyScript:
        {
            _status = CardStatus::Inert;
            _constructionTime = delay;
            break;
        }
        case CardCreationMethod::Manual:
        {
            if (type.hasAbility() || type.hasTargetAbility())
            {
                _status = CardStatus::Default;
            }
                
            _currentDelay = delay;
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
    _id = id;
}

const TurnType & Card::getConstructionTime() const
{
    return _constructionTime;
}

const TurnType & Card::getCurrentLifespan() const
{
    return _lifespan;
}

const TurnType & Card::getCurrentDelay() const
{
    return _currentDelay;
}

Card & Card::operator = (const Card & rhs)
{
    if (this != &rhs)
    {
        new (this) Card(rhs);
    }

    return *this;
}


const CardType & Card::getType() const
{
 return _type;
}

const PlayerID Card::getPlayer() const
{
    return _player;
}

const CardID Card::getID() const
{
 return _id;
}

bool Card::isInPlay() const
{
    return _inPlay;
}

bool Card::isFrozen() const
{
    return currentChill() >= currentHealth();
}

void Card::setInPlay(bool inPlay)
{
    _inPlay = inPlay;
}

bool Card::hasTarget() const
{
    return _hasTarget;
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
    return _dead;
}

const HealthType & Card::currentHealth() const
{
    return _currentHealth;
}

const HealthType & Card::currentChill() const
{
    return _currentChill;
}

const int & Card::getStatus() const
{
    return _status;
}

const int & Card::getAliveStatus() const
{
    return _aliveStatus;
}

bool Card::isBreachable() const
{
    return !isUnderConstruction() && !isDead();
}

bool Card::isOverkillable() const
{
    return isUnderConstruction() && !isDead();
}

bool Card::canBreachFor(const HealthType & damage) const
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

bool Card::canOverkillFor(const HealthType & damage) const
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

void Card::takeDamage(const HealthType & amount, const int & damageSource)
{
    _damageTaken = std::min(amount, _currentHealth);

    if (amount >= _currentHealth)
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
                _wasBreached = true;
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
        _currentHealth -= std::min(amount, _currentHealth);
        if (damageSource == DamageSource::Breach)
        {
            _wasBreached = true;
        }
    }
}

void Card::applyChill(const HealthType & amount)
{
    PRISMATA_ASSERT(currentChill() < currentHealth(), "We shouldn't be applying chill to a frozen card");

    _currentChill += amount;
}

void Card::removeChill(const HealthType & amount)
{
    PRISMATA_ASSERT(amount <= currentChill(), "Trying to remove too much chill from a card");

    _currentChill -= amount;
}

void Card::setStatus(int status)
{
    _status = status;
}

void Card::clearTarget()
{
    _hasTarget = false;
    _targetID = 0;
}

void Card::setTargetID(const CardID targetID)
{
    _hasTarget = true;
    _targetID = targetID;
}

const CardID Card::getTargetID() const
{
    PRISMATA_ASSERT(hasTarget(), "Trying to get the target of a card without one: %s", getType().getUIName().c_str() );

    return _targetID;
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

void Card::kill(const int & causeOfDeath)
{
    _dead = true;
    _aliveStatus = AliveStatus::KilledThisTurn;
    _causeOfDeath = causeOfDeath;
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
    PRISMATA_ASSERT(_wasBreached, "Can't un-breach a card that wasn't breached");

    if (getType().isFragile())
    {
        _currentHealth += _damageTaken;
    }
    else
    {
        _currentHealth = getType().getStartingHealth();
    }

    
    _wasBreached = false;
    _damageTaken = 0;
}

bool Card::isUnderConstruction() const
{
    return getConstructionTime() > 0;
}

bool Card::isDelayed() const
{
    return _currentDelay > 0;
}

const ChargeType & Card::getCurrentCharges() const
{
    return _currentCharges;
}

void Card::beginTurn()
{
    // card is no longer sellable
    _sellable = false;
    _damageTaken = 0;
    _wasBreached = false;
    _abilityUsedThisTurn = false;
    _killedCardIDs.clear();
    _createdCardIDs.clear();
    clearTarget();

    // update the alive status
    if (_aliveStatus == AliveStatus::KilledThisTurn)
    {
        _aliveStatus = AliveStatus::Dead;
        return;
    }

    // reduce lifespan
    if (!isUnderConstruction() && !isDelayed() && _lifespan > 0)
    {
        --_lifespan;
        
        if (_lifespan == 0)
        {
            kill(CauseOfDeath::Lifespan);
            return;
        }
    }

    // reduce delay
    if (!isUnderConstruction() && isDelayed())
    {
        --_currentDelay;
    }

    if (isDelayed())
    {
        setStatus(CardStatus::Inert);
    }

    // reduce construction time
    if (isUnderConstruction())
    {
        _constructionTime--;
    }
    
    // do everything else post-construction
    if (!isUnderConstruction() && !isDelayed())
    {
        // gain healthgained
        _currentHealth += _type.getHealthGained();
        if (_type.getHealthMax() > 0 && _currentHealth > _type.getHealthMax())
        {
            _currentHealth = _type.getHealthMax();
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

        _currentChill = 0;
    }
}

bool Card::canRunBeginOwnTurnScript() const
{
    return !isUnderConstruction() && _currentDelay == 0;
}

bool Card::canFrontlineFor(const HealthType & damage) const
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

    if (_currentDelay > 0)
    {
        return false;
    }

    if (_currentHealth < _type.getHealthUsed())
    {
        return false;
    }

    return true;
}

void Card::runAbilityScript()
{
    _currentDelay = _type.getAbilityScript().getDelay();

    if (_type.getAbilityScript().isSelfSac())
    {
        kill(CauseOfDeath::SelfSac);
    }
}

void Card::runBeginTurnScript()
{
    PRISMATA_ASSERT(canRunBeginOwnTurnScript(), "runBeginTurnScript() called when canRunBeginOwnTurnScript() is false");

    _currentDelay = _type.getBeginOwnTurnScript().getDelay();
}

bool Card::canUndoUseAbility() const
{
    if (_status != CardStatus::Assigned)
    {
        return false;
    }

    if (_dead && (_aliveStatus != AliveStatus::KilledThisTurn))
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
        _currentCharges += getType().getChargeUsed();
    }

    if (_type.getHealthUsed() > 0 && _currentHealth == 0)
    {
        _aliveStatus = AliveStatus::Alive;
        _causeOfDeath = CauseOfDeath::None;
    }

    _currentHealth += _type.getHealthUsed();

    if (_type.getAbilityScript().isSelfSac())
    {
        _aliveStatus = AliveStatus::Alive;
        _causeOfDeath = CauseOfDeath::None;
    }

    setStatus(CardStatus::Default);
    _abilityUsedThisTurn = false;
    _currentDelay = 0;
    _killedCardIDs.clear();
}

void Card::endTurn()
{
    _killedCardIDs.clear();
    _createdCardIDs.clear();
    clearTarget();

    PRISMATA_ASSERT(_createdCardIDs.size() == 0, "WTF");
}

void Card::useAbility()
{
    if (!canUseAbility())
    {
        bool b = canUseAbility();
    }

    PRISMATA_ASSERT(canUseAbility(), "useAbility() called when canUseAbility() is false: %s", getType().getName().c_str());

    _abilityUsedThisTurn = true;

    if (getType().usesCharges())
    {
        _currentCharges -= getType().getChargeUsed();
    }
        
    _currentHealth -= _type.getHealthUsed();

    if (_currentHealth == 0)
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

    if (condition.hasCardType() && condition.getTypeID() != 0 && condition.getTypeID() != _type.getID())
    {
        return false;
    }

    if (condition.isNotBlocking() && canBlock())
    {
        return false;
    }

    if (condition.isTech() && !_type.isTech())
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
    _createdCardIDs.push_back(id);
}

void Card::addKilledCardID(const CardID id)
{
    _killedCardIDs.push_back(id);
}

void Card::undoKill()
{
    PRISMATA_ASSERT(isDead(), "Can't undo kill on a non-dead card");

    _aliveStatus = AliveStatus::Alive;
    _dead = false;
    _causeOfDeath = CauseOfDeath::None;
}

const std::vector<CardID> & Card::getKilledCardIDs() const
{
    return _killedCardIDs;
}

const HealthType & Card::getDamageTaken() const
{
    return _damageTaken;
}

bool Card::wasBreached() const
{
    return _wasBreached;
}

bool Card::selfKilled() const
{
    return (_causeOfDeath == CauseOfDeath::SelfSac) || (_causeOfDeath == CauseOfDeath::SelfAbilityHealthCost);
}

bool Card::isSellable() const
{
    return _sellable;
}

const std::vector<CardID> & Card::getCreatedCardIDs() const
{
    return _createdCardIDs;
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

    if (_sellable)
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