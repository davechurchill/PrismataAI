#pragma once

#include "Common.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class CardType;
class CreateDescription
{
    
  
public:

    std::string         _cardName;
    mutable CardID      _typeID;
    CardID              _multiple;
    TurnType            _buildTime;
    TurnType            _lifespan;
    bool                _own;

    CreateDescription();
    CreateDescription(const std::string & cardName, bool bought);
    CreateDescription(const rapidjson::Value & value);
    
    const std::string toString() const;
    const std::string getCardName() const;
    const CardType getType() const;
    const CardID getTypeID() const;
    const CardID getMultiple() const;
    bool getOwn() const;
    const TurnType getBuildTime() const;
    const TurnType getLifespan() const;

    bool operator == (const CreateDescription & rhs) const;
};

}