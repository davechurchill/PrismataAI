#include "DestroyDescription.h"
#include "CardType.h"

using namespace Prismata;

DestroyDescription::DestroyDescription()
    : _typeID(0)
    , _multiple(1)
    , _own(true)
{
}


DestroyDescription::DestroyDescription(const rapidjson::Value & value)
    : DestroyDescription()
{
    PRISMATA_ASSERT(value.HasMember("cardName"), "DestroyDescription has no cardName");
    PRISMATA_ASSERT(value.HasMember("owner"), "DestroyDescription has no owner");
    
    _cardName = value["cardName"].GetString();

    if (strcmp(value["owner"].GetString(), "own") == 0) 
    { 
        _own = true; 
    }
    else if (strcmp(value["owner"].GetString(), "opponent") == 0) 
    { 
        _own = false; 
    }
    else 
    { 
        PRISMATA_ASSERT(false, "DestroyDescription(): Invalid owner."); 
    }

    if (value.HasMember("supply") && value["supply"].IsInt()) 
    { 
        _multiple = value["supply"].GetInt(); 
    }

    if (value.HasMember("condition") && value["condition"].IsObject())
    {
        _condition = Condition(value["condition"]);
    }
}

const Condition & DestroyDescription::getCondition() const
{
    return _condition;
}

bool DestroyDescription::getOwn() const
{
    return _own;
}

const std::string DestroyDescription::toString() const
{
    std::stringstream ss;

    ss << _cardName << " " << getTypeID() << " " << _multiple;

    return ss.str();
}

const CardID DestroyDescription::getTypeID() const
{
    if (_typeID == 0)
    {
        _typeID = CardTypes::GetCardType(_cardName).getID();
    }

    return _typeID;
}

const CardType DestroyDescription::getType() const
{
    return CardTypes::GetAllCardTypes()[getTypeID()];
}

const CardID DestroyDescription::getMultiple() const
{
    return _multiple;
}

bool DestroyDescription::operator == (const DestroyDescription & rhs) const
{
    if (_cardName != rhs._cardName) return false;
    if (_multiple != rhs._multiple) return false;
    if (_own != rhs._own) return false;
    if (!(_condition == rhs._condition)) return false;

    return true;
}