#pragma once

#include "Common.h"
#include "CardType.h"
#include "rapidjson/document.h"
#include <map>
#include "CardType.h"

namespace Prismata
{

enum { VAL_NONE, VAL_STRING, VAL_INT, VAL_BOOL, VAL_DOUBLE, VAL_VECTOR, VAL_OBJECT, VAL_CARDTYPE };

union genericValue_t
{
    int       _i;
    double    _d;
    bool      _b;
};

class GenericValue
{
    int                                 _valueType;
    
    genericValue_t                      _data;
    CardType                            _cardType;
    std::string                         _str;
    std::vector<GenericValue>           _vec;

public:

    GenericValue();
    GenericValue(const rapidjson::Value & v);
    GenericValue(const int i);
    GenericValue(const double d);
    GenericValue(const std::string & str);
    GenericValue(bool b);
    GenericValue(const CardType type);
    GenericValue(const std::vector<GenericValue> & v);

    const size_t size() const;

    const GenericValue & operator [] (const size_t & index) const;

    bool isCardType() const;
    bool isInt() const;
    bool isDouble() const;
    bool isBool() const;
    bool isString() const;
    bool isVector() const;

    const CardType getCardType() const;
    const int getInt() const;
    const double getDouble() const;
    bool getBool() const;
    const std::string & getString() const;
    const std::vector<GenericValue> & getVector() const;
};

}