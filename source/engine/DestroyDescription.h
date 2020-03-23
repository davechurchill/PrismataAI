#pragma once

#include "Common.h"
#include "Condition.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class CardType;
class DestroyDescription
{
    
public:

    std::string         _cardName;
    mutable CardID      _typeID;
    CardID              _multiple;
    bool                _own;
    Condition           _condition;

    DestroyDescription();
    DestroyDescription(const rapidjson::Value & value);
    
    const std::string toString() const;
    const CardType getType() const;
    const CardID getTypeID() const;
    const CardID getMultiple() const;
    bool getOwn() const;
    const Condition & getCondition() const;

    bool operator == (const DestroyDescription & rhs) const;
};

}