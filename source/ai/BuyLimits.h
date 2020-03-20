#pragma once

#include "Common.h"
#include "CardType.h"
#include "rapidjson/document.h"

namespace Prismata
{

class BuyLimits
{
    std::vector<CardID> _buyLimits;
    std::vector<bool>   _hasLimit;
    bool                                _hasAnyLimits;
    
public:

    BuyLimits();
    BuyLimits(const rapidjson::Value & val);

    const CardID & getLimit(const CardType & type) const;
    bool hasLimit(const CardType & type) const;
    bool hasAnyLimits() const;
    void setLimit(const CardType & index, const CardID limit);
    void addLimits(const BuyLimits & limits);
};
}