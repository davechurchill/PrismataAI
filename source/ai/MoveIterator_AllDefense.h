#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "MoveIterator.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{
 
class MoveIterator_AllDefense : public MoveIterator
{
    std::vector<IsomorphicCardSet>  _isoCardSets;
    std::vector<size_t>             _currentIsoIndex;
    size_t                          _currentFinalBlockerIndex;
    size_t                          _movesGenerated;
    size_t                          _legalMovesGenerated;

    void processIsomorphicCards();
    void incrementMove(size_t index = 0);
    bool isBlockingSequenceLegal() const;

public:
    
    MoveIterator_AllDefense(const PlayerID & playerID);
    
    void testIterator();

    virtual void            reset();
    virtual void            setState(const GameState & state);
    virtual void            getRandomMove(const GameState & state, Move & move);

    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription();
    virtual MoveIteratorPtr clone();
};

}