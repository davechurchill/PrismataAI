#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "MoveIterator.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{
 
class MoveIterator_All : public MoveIterator
{
    size_t                          _movesGenerated;
    size_t                          _legalMovesGenerated;

    std::vector<GameState>          _states;
    std::vector<Move>               _moves;
    std::vector<MoveIteratorPtr>    _iterators;

    void processFutureIterators(size_t index);
    void incrementMove(const size_t index);

public:
    
    MoveIterator_All(const PlayerID & playerID);
    
    virtual void            reset();
    virtual void            setState(const GameState & state);

    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription();
    virtual MoveIteratorPtr clone();
};

}