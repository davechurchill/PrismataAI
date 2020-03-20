#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "MoveIterator.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{
 
class MoveIterator_AllBuy : public MoveIterator
{
    std::vector<CardID>             _buyableCardIDs;
    std::vector<size_t>             _maxBuyable;
    std::vector<size_t>             _currentBuyIndex;
    size_t                          _movesGenerated;
    size_t                          _legalMovesGenerated;

    void incrementMove(size_t index = 0);
    void processBuyableCards();

public:
    
    MoveIterator_AllBuy(const PlayerID & playerID);
    
    void testIterator();

    virtual void            reset();
    virtual void            setState(const GameState & state);

    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription();
    virtual MoveIteratorPtr clone();
};

}