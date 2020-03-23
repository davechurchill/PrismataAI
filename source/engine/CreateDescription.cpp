#include "CreateDescription.h"
#include "CardType.h"

using namespace Prismata;

CreateDescription::CreateDescription()
    : _typeID(0)
    , _multiple(1)
    , _buildTime(0)
    , _lifespan(0)
    , _own(true)
{
}

CreateDescription::CreateDescription(const rapidjson::Value & value)
    : CreateDescription()
{
    _cardName = value[(rapidjson::SizeType)0].GetString();


    if (strcmp(value[(rapidjson::SizeType)1].GetString(), "own") == 0) 
    { 
        _own = true; 
    }
    else if (strcmp(value[(rapidjson::SizeType)1].GetString(), "opponent") == 0) 
    { 
        _own = false; 
    }
    else 
    { 
        PRISMATA_ASSERT(false, "CreateDescription(): Invalid owner."); 
    }

    if (value.Size() > 2 ) 
    { 
        _multiple = value[(rapidjson::SizeType)2].GetInt(); 
    }

    if (value.Size() > 3) 
    { 
        _buildTime = value[(rapidjson::SizeType)3].GetInt(); 
    }
    else 
    { 
        _buildTime = 1; 
    }

    if (value.Size() > 4) 
    { 
        _lifespan = value[(rapidjson::SizeType)4].GetInt(); 
    }

}

const TurnType CreateDescription::getBuildTime() const
{
    return _buildTime;
}

bool CreateDescription::getOwn() const
{
    return _own;
}

const std::string CreateDescription::toString() const
{
    std::stringstream ss;

    ss << _cardName << " " << getTypeID() << " " << _multiple << " " << _buildTime << " " << _lifespan;

    return ss.str();
}

const std::string CreateDescription::getCardName() const
{
    return _cardName;
}

const CardID CreateDescription::getTypeID() const
{
    if (_typeID == 0)
    {
        _typeID = CardTypes::GetCardType(_cardName).getID();
    }

    return _typeID;
}

const CardType CreateDescription::getType() const
{
    return CardTypes::GetAllCardTypes()[getTypeID()];
}

const CardID CreateDescription::getMultiple() const
{
    return _multiple;
}

const TurnType CreateDescription::getLifespan() const
{
    return _lifespan;
}

bool CreateDescription::operator == (const CreateDescription & rhs) const
{
    if (_cardName != rhs._cardName) return false;
    if (_multiple != rhs._multiple) return false;
    if (_buildTime != rhs._buildTime) return false;
    if (_lifespan != rhs._lifespan) return false;
    if (_own != rhs._own) return false;
    return true;
}