#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "Heuristics.h"
#include "PartialPlayer_ActionAbility_EconomyDefault.h"

namespace Prismata
{
 
class BuildOrderIterator 
{

protected:

    GameState                           _initialState;
    BuyLimits                           _buyLimits;
    PlayerID                            _buyPlayerID;
    PartialPlayer_ActionAbility_EconomyDefault _buyPlayer;
    Move                                _econMove;

    std::vector<CardID>                 _allowedBuyableIndex;
    std::vector< std::vector<Action> >  _actionStack;
    std::vector<CardID>                 _boughtCardIDStack;
    std::vector<CardID>                 _cardTypeCount;

    size_t                              _numBuys;
    size_t                              _nodesSearched;
    bool                                _print;

    void                                printStack() const;
    void                                passTurn(const GameState & state, const size_t turn);

public:
    
    BuildOrderIterator(const GameState & state);

    void setBuyLimits(const BuyLimits & buyLimits);
    void recurse(GameState & state, const CardID currentCardBuyableIndex, const size_t numBought, const size_t turn);
    size_t getNumBuys();
    size_t getNodesSearched();
        
    void solve();
    void debugSolve();

};
}