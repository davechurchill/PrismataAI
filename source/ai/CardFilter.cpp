#include "CardFilter.h"

using namespace Prismata;

CardFilter::CardFilter()
    : _name("DefaultFilter")
    , _allowResonate(false)
{

}

CardFilter::CardFilter(const std::string & name, const rapidjson::Value & filterVal)
    : _name(name)
    , _filter(CardTypes::GetAllCardTypes().size(), false)
    , _allowResonate(false)
{

    // set the default value for the filter if it exists
    if (filterVal.HasMember("default") && filterVal["default"].IsBool())
    {
        std::fill(_filter.begin(), _filter.end(), filterVal["default"].GetBool());
    }

    // detect allow resonate
    if (filterVal.HasMember("allowResonate") && filterVal["allowResonate"].IsBool())
    {
        _allowResonate = filterVal["allowResonate"].GetBool();
    }

    // add filters for card names
    if (filterVal.HasMember("cards"))
    {
        const rapidjson::Value & cardNames = filterVal["cards"];
        PRISMATA_ASSERT(cardNames.IsArray(), "cards is not an array");

        for (size_t i(0); i < cardNames.Size(); ++i)
        {
            PRISMATA_ASSERT(cardNames[i].IsString(), "cards element is not a String");

            if (CardTypes::CardTypeExists(cardNames[i].GetString()))
            {
                const CardType type = CardTypes::GetCardType(cardNames[i].GetString());
                _filter[type.getID()] = true;
            }
        }
    }

    // add filters for conditions
    if (filterVal.HasMember("conditions"))
    {
        const rapidjson::Value & conditions = filterVal["conditions"];
        PRISMATA_ASSERT(conditions.IsArray(), "Conditions is not an array");

        parseConditions(conditions);
    }

    if (filterVal.HasMember("stateConditions"))
    {
        const rapidjson::Value & conditions = filterVal["stateConditions"];
        PRISMATA_ASSERT(conditions.IsArray(), "Conditions is not an array");

        parseStateConditions(conditions);
    }
}

void CardFilter::addFilter(const CardFilter & filter)
{
    // add the card type filter by or-ing
    for (size_t i(0); i < CardTypes::GetAllCardTypes().size(); ++i)
    {
        _filter[i] = _filter[i] || filter[i];
    }

    // add all the conditions from the other filter
    for (size_t i(0); i < filter._conditions.size(); ++i)
    {
        _conditions.push_back(filter._conditions[i]);
    }

    // add all the state conditions from the other filter
    for (size_t i(0); i < filter._stateConditions.size(); ++i)
    {
        _stateConditions.push_back(filter._stateConditions[i]);
    }
}

bool CardFilter::operator [] (const CardType & type) const
{
    if (_filter.size() < type.getID())
    {
        return false;
    }

    return _filter[type.getID()];
}

bool CardFilter::getAllowResonate() const
{
    return _allowResonate;
}

// parse all the conditions
void CardFilter::parseConditions(const rapidjson::Value & conditions)
{
    for (size_t i(0); i < conditions.Size(); ++i)
    {
        PRISMATA_ASSERT(conditions[i].IsArray(), "Conditions element is not an Array");
        _conditions.push_back(std::vector<CardFilterCondition>());

        bool andConditions = true;
        for (size_t j(0); j < conditions[i].Size(); ++j)
        {
            //std::cout << i << "  " << j << " Reading new condition with size: " << conditions[i][j].Size() << "\n";
            _conditions[i].push_back(CardFilterCondition(conditions[i][j]));
        }
    }

    const std::vector<CardType> & allCardTypes = CardTypes::GetAllCardTypes();

    for (size_t c(0); c < allCardTypes.size(); ++c)
    {
        // skip if we've already filtered this type
        if (_filter[allCardTypes[c].getID()])
        {
            continue;
        }

        for (size_t i(0); i < _conditions.size(); ++i)
        {
            bool andConditions = true;
            for (size_t j(0); j < _conditions[i].size(); ++j)
            {                
                if (!_conditions[i][j].evaluate(allCardTypes[c]))
                {
                    andConditions = false;
                    break;
                }
            }

            // if the condition array matches the card, set the filter to true
            if (andConditions)
            {
                _filter[allCardTypes[c].getID()] = true;
            }
        }
    }
}

void CardFilter::parseStateConditions(const rapidjson::Value & conditions)
{
    for (size_t i(0); i < conditions.Size(); ++i)
    {
        PRISMATA_ASSERT(conditions[i].IsArray(), "Conditions element is not an Array");
        _stateConditions.push_back(std::vector<CardFilterCondition>());

        bool andConditions = true;
        for (size_t j(0); j < conditions[i].Size(); ++j)
        {
            //std::cout << i << "  " << j << " Reading new condition with size: " << conditions[i][j].Size() << "\n";
            _stateConditions[i].push_back(CardFilterCondition(conditions[i][j]));
        }
    }
}

bool CardFilter::evaluate(const GameState & state, const CardType & type) const
{
    if (_filter[type.getID()])
    {
        return true;
    }

    for (size_t i(0); i < _stateConditions.size(); ++i)
    {
        bool andConditions = true;
        for (size_t j(0); j < _stateConditions[i].size(); ++j)
        {                
            if (!_stateConditions[i][j].evaluate(state, type))
            {
                andConditions = false;
                break;
            }
        }

        if (andConditions)
        {
            return true;
        }
    }

    return false;
}

void CardFilter::printFilter() const
{
    std::cout << "\nFilter Name: " << _name << "\n";

    const std::vector<CardType> & allCardTypes = CardTypes::GetAllCardTypes();

    for (size_t i(0); i < allCardTypes.size(); ++i)
    {
        if (_filter[i])
        {
            std::cout << "   " << allCardTypes[i].getUIName() << "\n";
        }
    }
}