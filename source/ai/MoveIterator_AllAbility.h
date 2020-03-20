#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "MoveIterator.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{
 
class MoveIterator_AllAbility : public MoveIterator
{
    std::vector<IsomorphicCardSet>  _isoCardSets;
    std::vector<size_t>             _currentIsoIndex;
    size_t                          _movesGenerated;
    size_t                          _legalMovesGenerated;
    size_t                          _currentMove;
    GameState                       _currentState;

    std::vector<Move>               _allMoves;
    Move                            _actionStack;

    bool isTerminalState(size_t isoIndex);
    bool checkSolution();
    void generateAllMoves();
    void recurse(size_t isoIndex);
    void processIsomorphicCards();
    void incrementMove(size_t index = 0);

public:
    
    MoveIterator_AllAbility(const PlayerID & playerID);
    
    void testIterator();

    virtual void            reset();
    virtual void            setState(const GameState & state);

    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription();
    virtual MoveIteratorPtr clone();
};

class MoveLengthComparator 
{
    

public:

    MoveLengthComparator()
    {
    }

    bool operator() (const Move & m1, const Move & m2) const
    {
        return m1.size() > m2.size();
    }
};

}