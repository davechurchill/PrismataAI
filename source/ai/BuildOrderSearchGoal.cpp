#include "BuildOrderSearchGoal.h"

using namespace Prismata;

BuildOrderSearchGoal::BuildOrderSearchGoal()
{
}

BuildOrderSearchGoal::BuildOrderSearchGoal(const rapidjson::Value & val)
{
    PRISMATA_ASSERT(val.IsArray(), "Goal json must be an array");

    for (size_t i(0); i < val.Size(); ++i)
    {
        const rapidjson::Value & limit = val[i];

        PRISMATA_ASSERT(limit.IsArray() && limit.Size() == 3, "Goal array entry must be an array of length 2 or 3");
        PRISMATA_ASSERT(limit[0u].IsString() && limit[1u].IsInt() && limit[1u].IsInt(), "Goal array entry must be [\"CardName\", Integer, Integer]");

        const std::string & cardName = limit[0u].GetString();
        const int cardLimit = limit[1u].GetInt();
        const int turn = limit[2u].GetInt();
        if (CardTypes::CardTypeExists(cardName))
        {
            addGoal(CardTypes::GetCardType(cardName), cardLimit, turn);
        }
        else
        {
            std::cout << "Warning: CardType does not exist: " << cardName << "\n";
        }
    }
}

const size_t BuildOrderSearchGoal::size() const
{
    return _goalType.size();
}

void BuildOrderSearchGoal::addGoal(const CardType & type, const size_t goalAmount, const size_t goalTurn)
{
    _goalType.push_back(type);
    _goalAmount.push_back(goalAmount);
    _goalTurn.push_back(goalTurn);
}

bool BuildOrderSearchGoal::meetsGoal(const GameState & state) const
{
    const PlayerID player = Players::Player_One;
    for (size_t i(0); i < size(); ++i)
    {
        const CardType & type = _goalType[i];
        if (state.numCardsOfType(player, type) < _goalAmount[i])
        {
            return false;
        }
    }

    return true;
}

bool BuildOrderSearchGoal::cannotMeetGoal(const GameState & state) const
{
    const PlayerID player = Players::Player_One;
    for (size_t i(0); i < size(); ++i)
    {
        const CardType & type = _goalType[i];

        if (_goalTurn[i] == 0)
        {
            continue;
        }

        if (state.getTurnNumber() < _goalTurn[i]*2)
        {
            continue;
        }

        if (state.numCardsOfType(player, type) >= _goalAmount[i])
        {
            continue;
        }

        return true;
    }

    return false;
}