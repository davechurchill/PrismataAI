#pragma once

#include "Common.h"
#include "GameState.h"

#include "BuyLimits.h"
#include "Heuristics.h"

namespace Prismata
{
 
class BuyIterator 
{

protected:

    GameState                           _state;
    BuyLimits                           _buyLimits;

    std::vector<CardID>                 _allowedBuyableIndex;
    std::vector<Action>                 _actionStack;
    std::vector<CardID>                 _boughtCardIDStack;
    std::vector<CardID>                 _cardTypeCount;

    size_t                              _numBuys;
    size_t                              _nodesSearched;
    bool                                _print;

    void printStack() const;

public:
    
    BuyIterator(const GameState & state);

    void setBuyLimits(const BuyLimits & buyLimits);
    void recurse(const CardID currentCardBuyableIndex, const size_t numBought);
    size_t getNumBuys();
    size_t getNodesSearched();
        
    void solve();
    void debugSolve();

};
}