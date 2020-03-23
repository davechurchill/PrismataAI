#pragma once

#include "Common.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class CardType;
class SacDescription
{
    std::string         _cardName;
    mutable CardID      _typeID;
    CardID              _multiple;

public:

    SacDescription();
    SacDescription(const rapidjson::Value & value);
    
    const CardID getMultiple() const;
    const CardType getType() const;
    const CardID getTypeID() const;
    const std::string & getCardName() const;

    bool operator == (const SacDescription & rhs) const;
};

}