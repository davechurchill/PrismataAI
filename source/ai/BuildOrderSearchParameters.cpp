#include "BuildOrderSearchParameters.h"

using namespace Prismata;

BuildOrderSearchParameters::BuildOrderSearchParameters()
{

}

BuildOrderSearchParameters::BuildOrderSearchParameters(const rapidjson::Value & val)
    : _timeLimitMS(0)
    , _maxTurns(0)
    , _printStack(false)
{
    PRISMATA_ASSERT(val.IsObject(), "Params json must be an object");

    // parse the name
    if (val.HasMember("name") && val["name"].IsString())
    {
        _name = val["name"].GetString();
    }
    
    PRISMATA_ASSERT(val.HasMember("type") && val["type"].IsString(), "BuildOrderSearchParameters must have a type string");
    const std::string & type = val["type"].GetString();

    if (type == "EnumerateBuys")
    {
        _searchType = BuildOrderSearchTypes::EnumerateBuys;
        
        parseTimeLimit(val);
        parseState(val);
        parseRelevantCards(val);
        parseBuyLimits(val);
        parseMaxTurns(val);
        parsePrintStack(val);
    }
    else if (type == "UnitGoal")
    {
        _searchType = BuildOrderSearchTypes::UnitGoal;

        parseTimeLimit(val);
        parseState(val);
        parseRelevantCards(val);
        parseGoal(val);
        //parseMaxTurns(val);
        parseBuyLimits(val);
        parsePrintStack(val);
    }
    else
    {
        std::cout << "Warning: Unknown build order search type: " << type << "\n";
    }
}

const BuildOrderSearchGoal & BuildOrderSearchParameters::getGoal() const
{
    return _goal;
}

const size_t & BuildOrderSearchParameters::getSearchType() const
{
    return _searchType;
}

const BuyLimits & BuildOrderSearchParameters::getBuyLimits() const
{
    return _buyLimits;
}

const size_t & BuildOrderSearchParameters::getMaxTurns() const
{
    return _maxTurns;
}

const double & BuildOrderSearchParameters::getTimeLimitMS() const
{
    return _timeLimitMS;
}

const GameState & BuildOrderSearchParameters::getInitialState() const
{
    return _initialState;
}

bool BuildOrderSearchParameters::printStack() const
{
    return _printStack;
}

void BuildOrderSearchParameters::parseState(const rapidjson::Value & root)
{
    GameState state;
    
    PRISMATA_ASSERT(root.HasMember("state") && root["state"].IsObject(), "BuildOrderSearchParameters must have a starting state in JSON object form");
    const rapidjson::Value & val = root["state"];

    if (val.HasMember("startingResources") && val["startingResources"].IsString())
    {
        Resources startingMana(val["startingResources"].GetString());
        state.manuallySetMana(Players::Player_One, startingMana);
    }
    else
    {
        std::cout << "State must have 'startingResources' string\n";
    }

    if (val.HasMember("startingUnits") && val["startingUnits"].IsArray())
    {
        PRISMATA_ASSERT(val["startingUnits"].IsArray(), "Goal json must be an array");

        for (size_t i(0); i < val["startingUnits"].Size(); ++i)
        {
            const rapidjson::Value & card = val["startingUnits"][i];

            PRISMATA_ASSERT(card.IsArray() && card.Size() == 2, "startingUnits array entry must be an array of length 2");
            PRISMATA_ASSERT(card[0u].IsString() && card[1u].IsInt(), "startingUnits array entry must be [\"CardName\", Integer]");

            const std::string & cardName = card[0u].GetString();
            const int amount = card[1u].GetInt();
            if (CardTypes::CardTypeExists(cardName))
            {
                state.addCard(state.getActivePlayer(), CardTypes::GetCardType(cardName), amount, CardCreationMethod::Manual, 0, 0);
            }
            else
            {
                std::cout << "Warning: CardType does not exist: " << cardName << "\n";
            }
        }
    }
    else
    {
        std::cout << "State must have 'startingUnits' array\n";
    }

    if (val.HasMember("buyableUnits") && val["buyableUnits"].IsArray())
    {
        // parse the relevant actions
        if (val.HasMember("buyableUnits") && val["buyableUnits"].IsArray())
        for (size_t i(0); i < val["buyableUnits"].Size(); ++i)
        {
            const rapidjson::Value & buyable = val["buyableUnits"][i];

            PRISMATA_ASSERT(buyable.IsString(), "buyableUnits entry must be a string");

            const std::string & cardName = buyable.GetString();
            if (CardTypes::CardTypeExists(cardName))
            {
                state.addCardBuyable(CardTypes::GetCardType(cardName));
            }
            else
            {
                std::cout << "Warning: buyableUnits CardType does not exist: " << cardName << "\n";
            }
        }
    }
    else
    {
        std::cout << "State must have 'buyableUnits' array\n";
    }

    _initialState = state;
}

bool BuildOrderSearchParameters::isRelevant(const CardType type) const
{
    // if we don't have any relevant actions then we don't care, do it's relevant
    if (_relevantCards.empty())
    {
        return true;
    }

    // otherwise return whether the vector contains the card type
    return std::find(std::begin(_relevantCards), std::end(_relevantCards), type) != std::end(_relevantCards);
}

void BuildOrderSearchParameters::parseRelevantCards(const rapidjson::Value & root)
{
    // parse the relevant actions
    if (root.HasMember("relevant") && root["relevant"].IsArray())
    for (size_t i(0); i < root["relevant"].Size(); ++i)
    {
        const rapidjson::Value & relevantType = root["relevant"][i];

        PRISMATA_ASSERT(relevantType.IsString(), "Relevant type must be a string");

        const std::string & cardName = relevantType.GetString();
        if (CardTypes::CardTypeExists(cardName))
        {
            _relevantCards.push_back(CardTypes::GetCardType(cardName));
        }
        else
        {
            std::cout << "Warning: Relevant CardType does not exist: " << cardName << "\n";
        }
    }
}

void BuildOrderSearchParameters::parsePrintStack(const rapidjson::Value & root)
{
    if (root.HasMember("printStack") && root["printStack"].IsBool())
    {
        _printStack = root["printStack"].GetBool();
    }
}

void BuildOrderSearchParameters::parseBuyLimits(const rapidjson::Value & root)
{
    if (root.HasMember("buyLimits") && root["buyLimits"].IsArray())
    {
        _buyLimits = BuyLimits(root["buyLimits"]);
    }
}

void BuildOrderSearchParameters::parseMaxTurns(const rapidjson::Value & root)
{
    PRISMATA_ASSERT(root.HasMember("maxTurns") && root["maxTurns"].IsInt(), "EnumerateBuys must have a maxTurns int");
    _maxTurns = root["maxTurns"].GetInt();
}

void BuildOrderSearchParameters::parseGoal(const rapidjson::Value & root)
{
    PRISMATA_ASSERT(root.HasMember("goal") && root["goal"].IsArray(), "BuildOrderSearchParameters must have a 'goal' array");
    _goal = BuildOrderSearchGoal(root["goal"]);
}

void BuildOrderSearchParameters::parseTimeLimit(const rapidjson::Value & root)
{
    if (root.HasMember("timeLimitMS") && root["timeLimitMS"].IsInt())
    {
        _timeLimitMS = root["timeLimitMS"].GetInt();
    }
}