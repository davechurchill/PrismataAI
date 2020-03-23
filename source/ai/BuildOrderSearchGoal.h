#pragma once

#include "Common.h"
#include "BuyLimits.h"
#include "GameState.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class BuildOrderSearchGoal
{
    std::vector<CardType>   _goalType;
    std::vector<size_t>     _goalAmount;
    std::vector<size_t>     _goalTurn;

public:

    BuildOrderSearchGoal();
    BuildOrderSearchGoal(const rapidjson::Value & val);

    const size_t size() const;
    void addGoal(const CardType type, const size_t goalAmount, const size_t goalTurn);
    bool meetsGoal(const GameState & state) const;
    bool cannotMeetGoal(const GameState & state) const;
};

}