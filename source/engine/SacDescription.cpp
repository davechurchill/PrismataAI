#include "SacDescription.h"
#include "CardType.h"

using namespace Prismata;

SacDescription::SacDescription()
    : _multiple(1)
    , _typeID(0)
{
}

SacDescription::SacDescription(const rapidjson::Value & value)
    : SacDescription()
{
    PRISMATA_ASSERT(value.IsArray(), "SacDescription is not an array");
    _cardName = value[(rapidjson::SizeType)0].GetString();

    if (value.Size() > 1 ) 
    { 
        PRISMATA_ASSERT(value[(rapidjson::SizeType)1].IsInt(), "SacDescription 2nd element should be int");
        _multiple = value[(rapidjson::SizeType)1].GetInt(); 
    }

    //std::cout << "Sac " << multiplicity << " " << cardName << std::endl;
}

const CardID SacDescription::getTypeID() const
{
    if (_typeID == 0)
    {
        _typeID = CardTypes::GetCardType(_cardName).getID();
    }

    return _typeID;
}

const CardType SacDescription::getType() const
{
    return CardTypes::GetAllCardTypes()[getTypeID()];
}

const CardID SacDescription::getMultiple() const
{
    return _multiple;
}

const std::string & SacDescription::getCardName() const
{
    return _cardName;
}

bool SacDescription::operator == (const SacDescription & rhs) const
{
    if (_cardName != rhs._cardName) return false;
    if (_multiple != rhs._multiple) return false;

    return true;
}