#include "GenericValue.h"

using namespace Prismata;
GenericValue::GenericValue() 
    : _valueType(VAL_NONE) 
{
}

GenericValue::GenericValue(const rapidjson::Value & v)
{
    if (v.IsInt())
    {
        _valueType = VAL_INT;
        _data._i = v.GetInt();
        //std::cout << _data._i << "\n";
    }
    else if (v.IsBool())
    {   
        _valueType = VAL_BOOL;
        _data._b = v.GetBool();
        //std::cout << _data._b << "\n";
    }
    else if (v.IsDouble())
    {
        _valueType = VAL_DOUBLE;
        _data._d = v.GetDouble();
        //std::cout << _data._d << "\n";
    }
    else if (v.IsArray())
    {
        _valueType = VAL_VECTOR;
        for (size_t i(0); i < v.Size(); ++i)
        {
            _vec.push_back(GenericValue(v[i]));
        }
        //std::cout << "array" << "\n";
    }
    else if (v.IsString())
    {
        _valueType = VAL_STRING;
        _str = v.GetString();
        //std::cout << _str << "\n";
    }
    else
    {
        PRISMATA_ASSERT(false, "Type not implemented in GenericValue");
    }
}

GenericValue::GenericValue(const int i) 
    : _valueType(VAL_INT) { _data._i = i; }
    
GenericValue::GenericValue(const double d) 
    : _valueType(VAL_DOUBLE) { _data._d = d; }

GenericValue::GenericValue(const std::string & str) 
    : _valueType(VAL_STRING), _str(str) {}

GenericValue::GenericValue(bool b) 
    : _valueType(VAL_BOOL) { _data._b = b; }

GenericValue::GenericValue(const std::vector<GenericValue> & v) 
    : _valueType(VAL_VECTOR), _vec(v) {}

GenericValue::GenericValue(const CardType type)
    : _valueType(VAL_CARDTYPE), _cardType(type) {}

const size_t GenericValue::size() const { return _vec.size(); }

const GenericValue & GenericValue::operator [] (const size_t & index) const
{
    return _vec[index];
}

bool GenericValue::isCardType() const { return _valueType == VAL_CARDTYPE; }
bool GenericValue::isInt() const      { return _valueType == VAL_INT; }
bool GenericValue::isDouble() const   { return _valueType == VAL_DOUBLE; }
bool GenericValue::isBool() const     { return _valueType == VAL_BOOL; }
bool GenericValue::isString() const   { return _valueType == VAL_STRING; }
bool GenericValue::isVector() const   { return _valueType == VAL_VECTOR; }

const CardType GenericValue::getCardType() const { return _cardType; }
const int GenericValue::getInt() const { return _data._i; }
const double GenericValue::getDouble() const { return _data._d; }
bool GenericValue::getBool() const { return _data._b; }
const std::string & GenericValue::getString() const { return _str; }
const std::vector<GenericValue> & GenericValue::getVector() const { return _vec; }
