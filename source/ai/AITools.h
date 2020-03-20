#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "AllPlayers.h"

namespace Prismata
{

namespace AITools
{
    std::string GetClickString(const Move & m, const GameState & state);
    std::string GetClickString(const Action & m, const GameState & state);
    std::string GetTypeString(const PlayerID player, const GameState & state);
    Move GetMoveFromClickString(const std::string & clickString, const PlayerID player, const GameState & state);
    Action GetActionFromClickJSON(const rapidjson::Value & click, const PlayerID player, const GameState & state, const std::string & clickString, const GameState & originalState);
    int FindIsomorphicCardID(const Card & card, const GameState & state);

    void PerformAIError(const std::string & errorType);

    bool PlayerShouldResign(const GameState & state, const PlayerID & playerID);
    std::string GetAIMove(const std::string & aiParamsString);
    std::string InitializeAI(const std::string & initString);
    std::string InitializeAIAndGetAIMove(const std::string & inputString);
    GameState GetStateFromInitString(const std::string & inputString);

    void PredictEnemyNextTurn(GameState & state, bool solveDefense = true);
    double CalculateEnemyNextTurnDefenseLoss(GameState & state);
    PPPtr GetPredictionPlayer(const PlayerID & player);

    double CalculateWipeoutLoss(GameState & state, const PlayerID player);

    bool PurchaseIsOutOfSync(const PlayerID player, const CardType & type, const GameState & state);

    size_t NumResonatorsReady(const CardType & type, const GameState & state, const PlayerID & player, const TurnType maxConstructionTime);
    size_t NumResonateesReady(const CardType & type, const GameState & state, const PlayerID & player, const TurnType maxConstructionTime);
    Resources GetReceiveFromResonators(const CardType & type, const GameState & state, const PlayerID & player, const TurnType maxConstructionTime);
    Resources GetReceiveFromResonatees(const CardType & type, const GameState & state, const PlayerID & player, const TurnType maxConstructionTime);
    
    void TestParseJSONString(const std::string & jsonString);

    Move StripUndoActions(const Move & m, const GameState & state);

    template <class T, class F>
    void SortVector(std::vector<T> & v, const F & f)
    {
        for (size_t i(0); i < v.size(); ++i)
        {
            for (size_t j(i+1); j < v.size(); ++j)
            {
                size_t size = v.size();
                //std::cout << "Size is " << size << ", calling functor\n";
                if (f(v[j], v[i]))
                {
                    size_t newSize = v.size();
                    //std::cout << "New size is " << newSize << "\n";
                    PRISMATA_ASSERT(size == newSize, "Sizes don't match");
                    std::swap(v[i], v[j]);
                }
            }
        }
    }
}
}
