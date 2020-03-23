#include "CardFilterCondition.h"

using namespace Prismata;

CardFilterCondition::CardFilterCondition(const rapidjson::Value & condVal)
    : _OP(0)
{
    PRISMATA_ASSERT(condVal.IsArray(), "ConditionFilter must be an array");

    const std::string & property = condVal[0].GetString();
    const std::string & op = condVal[1].GetString();

    // store the condition property
    if      (property == "lifespan")            { _LHS = GenericValue(COND_LIFESPAN);             _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "constructionTime")    { _LHS = GenericValue(COND_CONSTRUCTIONTIME);     _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "isBlocker")           { _LHS = GenericValue(COND_ISBLOCKER);            _RHS = GenericValue(condVal[2].GetBool());}
    else if (property == "hasAbility")          { _LHS = GenericValue(COND_HASABILITY);           _RHS = GenericValue(condVal[2].GetBool());}
    else if (property == "hasBeginTurnAbility") { _LHS = GenericValue(COND_HASBEGINTURNABILITY);  _RHS = GenericValue(condVal[2].GetBool());}
    else if (property == "attackProduced")      { _LHS = GenericValue(COND_ATTACKPRODUCED);       _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "startingHealth")      { _LHS = GenericValue(COND_STARTINGHEALTH);       _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "abilityAttack")       { _LHS = GenericValue(COND_ABILITYATTACK);        _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "beginTurnAttack")     { _LHS = GenericValue(COND_BEGINTURNATTACK);      _RHS = GenericValue(condVal[2].GetInt());}
    else if (property == "cardName")            
    { 
        _LHS = GenericValue(COND_CARDNAME);

        const std::string & s = condVal[2].GetString();
        _RHS = GenericValue(CardTypes::CardTypeExists(s) ? CardTypes::GetCardType(s) : CardTypes::None);
    }
    else if (condVal[0].IsString())
    {
        if (CardTypes::CardTypeExists(condVal[0].GetString()))
        {
            _LHS = GenericValue(CardTypes::GetCardType(condVal[0].GetString()));
                        
            if (condVal[2].IsInt())
            {
                _RHS = GenericValue(condVal[2].GetInt());
            }
            else if (condVal[2].IsString())
            {
                const std::string & s2 = condVal[2].GetString();
                _RHS = GenericValue(CardTypes::CardTypeExists(s2) ? CardTypes::GetCardType(s2) : CardTypes::None);
            }
        }
    }

    // store the operation 
    if      (op == "=")     { _OP = OP_EQ; }
    else if (op == "!=")    { _OP = OP_NEQ; }
    else if (op == "<")     { _OP = OP_LT; }
    else if (op == ">")     { _OP = OP_GT; }
    else if (op == "<=")    { _OP = OP_LEQ; }
    else if (op == ">=")    { _OP = OP_GEQ; }
}

bool CardFilterCondition::evaluate(const CardType type) const
{
    PRISMATA_ASSERT(_LHS.isInt(), "LHS should be int for card type evaluations");

    switch(_LHS.getInt())
    {
        case COND_LIFESPAN:             return ConditionBinaryOperator(type.getLifespan(),              _OP, _RHS.getInt());
        case COND_CONSTRUCTIONTIME:     return ConditionBinaryOperator(type.getConstructionTime(),      _OP, _RHS.getInt());
        case COND_ISBLOCKER:            return ConditionBinaryOperator(type.canBlock(false),            _OP, _RHS.getBool());
        case COND_HASABILITY:           return ConditionBinaryOperator(type.hasAbility(),               _OP, _RHS.getBool());
        case COND_HASBEGINTURNABILITY:  return ConditionBinaryOperator(type.hasBeginOwnTurnScript(),    _OP, _RHS.getBool());
        case COND_ATTACKPRODUCED:       return ConditionBinaryOperator(type.getAttack(),                _OP, _RHS.getInt());
        case COND_STARTINGHEALTH:       return ConditionBinaryOperator(type.getStartingHealth(),        _OP, _RHS.getInt());
        case COND_ABILITYATTACK:        return ConditionBinaryOperator(type.getAbilityAttackAmount(),   _OP, _RHS.getInt());
        case COND_BEGINTURNATTACK:      return ConditionBinaryOperator(type.getBeginTurnAttackAmount(), _OP, _RHS.getInt());
        default:
        {
            PRISMATA_ASSERT(false, "Unknown _LHS: %d", _LHS.getInt());
        
        }
    }

    return false;
}

bool CardFilterCondition::evaluate(const GameState & state, const CardType type) const
{
    // if the LHS is a card type, we're doing an operation on how many of them we have
    if (_LHS.isCardType())
    {
        PRISMATA_ASSERT(_RHS.isInt() || _RHS.isCardType(), "RHS Should be card type or int");

        int numType1 = state.numCardsOfType(state.getActivePlayer(), _LHS.getCardType());
        int numType2 = _RHS.isInt() ? _RHS.getInt() : state.numCardsOfType(state.getActivePlayer(), _RHS.getCardType());
        
        return ConditionBinaryOperator(numType1, _OP, numType2);
    }
    // otherwise the lHS is an integer conditional property
    else if (_LHS.isInt())
    {
        if (_LHS.getInt() == COND_CARDNAME)
        {
            return ConditionBinaryOperator(type.getID(), _OP, _RHS.getCardType().getID());
        }

        PRISMATA_ASSERT(false, "Unknown state condition property: %d", _LHS.getInt());
    }

    PRISMATA_ASSERT(false, "Unknown state condition state");
    return false;
}