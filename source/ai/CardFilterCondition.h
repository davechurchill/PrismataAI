#pragma once

#include "Common.h"
#include "CardType.h"
#include "rapidjson/document.h"
#include "GenericValue.h"
#include "GameState.h"

namespace Prismata
{
    
enum {  COND_PROPERTY,
        COND_LIFESPAN, 
        COND_ISBLOCKER, 
        COND_HASABILITY, 
        COND_HASBEGINTURNABILITY, 
        COND_STARTINGHEALTH,
        COND_CONSTRUCTIONTIME,
        COND_ATTACKPRODUCED,
        COND_ABILITYATTACK,
        COND_BEGINTURNATTACK,
        COND_CARDNAME, 
        COND_CARDTYPE};

enum {  OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LEQ, OP_GEQ };

class CardFilterCondition
{
    int             _OP;
    GenericValue    _LHS;
    GenericValue    _RHS;

    bool            _valid;

    bool evalRecurse(const GenericValue & value, const CardType type);
    bool evalSingle(const GenericValue & value, const CardType type);

public:

    CardFilterCondition(const rapidjson::Value & condVal);

    bool evaluate(const CardType type) const;
    bool evaluate(const GameState & state, const CardType type) const;
};

template <class T, class U>
bool ConditionBinaryOperator(const T & t, const int & op, const U & u)
{
    switch(op)
    {
        case OP_EQ:     return t == u;
        case OP_NEQ:    return t != u;
        case OP_LT:     return t < u;
        case OP_GT:     return t > u;
        case OP_LEQ:    return t <= u;
        case OP_GEQ:    return t >= u;
        default:
        {
            PRISMATA_ASSERT(false, "Unknown Operator: %d", op);
        
        }
    }

    return false;
}
}