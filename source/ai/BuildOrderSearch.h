#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "Heuristics.h"
#include "BuildOrderSearchParameters.h"
#include "PartialPlayer_ActionAbility_EconomyDefault.h"

namespace Prismata
{
 
class BuildOrderSearch 
{

protected:

    BuildOrderSearchParameters          _params;
    PartialPlayer_ActionAbility_EconomyDefault _buyPlayer;
    Move                                _econMove;

    std::vector<CardID>                 _allowedBuyableIndex;
    std::vector< std::vector<Action> >  _actionStack;
    std::vector<CardID>                 _boughtCardIDStack;
    std::vector<CardID>                 _cardTypeCount;

    CardType                            _droneType;
    size_t                              _numBuys;
    size_t                              _nodesSearched;
    bool                                _solutionFound;
    size_t                              _bestSolutionTurn;
    size_t                              _currentMaxTurn;
    size_t                              _bestSolutionMaxDrones;
    std::string                         _bestSolutionString;
    
    void                                passTurn(const GameState & state, const size_t turn);
    void                                updateSolution(const GameState & state, const size_t & turn);
    std::string                         getStackString(const size_t & turn) const;
    bool                                isTerminalNode(const GameState & state, const CardID currentCardBuyableIndex, const size_t & turn);

public:
    
    BuildOrderSearch(const BuildOrderSearchParameters & params);

    void recurse(GameState & state, const CardID currentCardBuyableIndex, const size_t numBought, const size_t turn);
    size_t getNumBuys();
    size_t getNodesSearched();
        
    void solve();

    static void DoBuildOrderSearch(const rapidjson::Value & val); 
    static void ParseBuildOrderSearch(const std::string & json);
};
}