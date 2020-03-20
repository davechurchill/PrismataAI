#pragma once

#include "Common.h"
#include "BuyLimits.h"
#include "GameState.h"
#include "AIParameters.h"
#include "BuildOrderSearchGoal.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

namespace BuildOrderSearchTypes
{
    enum { EnumerateBuys, UnitGoal, Size};
}

class BuildOrderSearchParameters
{
    size_t                      _searchType;
    std::string                 _name;
    PlayerID                    _buyPlayer;
    GameState                   _initialState;
    BuildOrderSearchGoal        _goal;
    BuyLimits                   _buyLimits;
    std::vector<CardType>       _relevantCards;
    double                      _timeLimitMS;
    size_t                      _maxTurns;
    bool                        _printStack;

    void parseState(const rapidjson::Value & root);
    void parseRelevantCards(const rapidjson::Value & root);
    void parseBuyLimits(const rapidjson::Value & root);
    void parseMaxTurns(const rapidjson::Value & root);
    void parseGoal(const rapidjson::Value & root);
    void parseTimeLimit(const rapidjson::Value & root);
    void parsePrintStack(const rapidjson::Value & root);

public:

    BuildOrderSearchParameters();
    BuildOrderSearchParameters(const rapidjson::Value & val);

    const size_t &                  getMaxTurns() const;
    const size_t &                  getSearchType() const;
    const GameState &               getInitialState() const;
    const BuildOrderSearchGoal &    getGoal() const;
    const BuyLimits &               getBuyLimits() const;
    const double &                  getTimeLimitMS() const;
    bool                      isRelevant(const CardType & type) const;
    bool                      printStack() const;
};

}