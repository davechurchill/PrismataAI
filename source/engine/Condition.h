#pragma once

#include "Common.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class CardType;
class Condition
{
    
public:

    std::string         _cardName;
    mutable CardID      _typeID;
    HealthType          _healthAtMost;
    bool                _isTech;
    bool                _notBlocking;
    bool                _hasHealthCondition;

    Condition();
    Condition(const rapidjson::Value & value);
    
    const std::string toString() const;
    const CardType getType() const;
    const CardID getTypeID() const;
    bool isTech() const;
    bool isNotBlocking() const;
    bool hasCardType() const;
    bool hasHealthCondition() const;
    const HealthType getHealthAtMost() const;

    bool operator == (const Condition & rhs) const;
};

}