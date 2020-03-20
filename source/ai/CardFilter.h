#pragma once

#include "Common.h"
#include "CardType.h"
#include "rapidjson/document.h"
#include "CardFilterCondition.h"

#define STATE_CONDITION 0
#define CARDTYPE_CONDITION 1
#define CARD_CONDITION 2

namespace Prismata
{

class CardFilter
{
    std::string                         _name;                              // name of the filter
    bool                                _allowResonate;                     // whether or not to allow resonating cards over all other conditions
    std::vector<bool>                   _filter;                            // bool filter vector where index = cardTypeID
    std::vector< std::vector<CardFilterCondition> >    _conditions;         // conditions for card types not reliant on state
    std::vector< std::vector<CardFilterCondition> >    _stateConditions;    // conditions for card types reliant on state
    
    void parseConditions(const rapidjson::Value & condVal);
    void parseStateConditions(const rapidjson::Value & condVal);

public:

    CardFilter();
    CardFilter(const std::string & name, const rapidjson::Value & filterVal);
    
    bool operator [] (const CardType & type) const;
    bool evaluate(const GameState & state, const CardType & type) const;
    bool getAllowResonate() const;

    void printFilter() const;
    void addFilter(const CardFilter & filter);
};

}