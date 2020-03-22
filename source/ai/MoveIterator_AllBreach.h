#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"
#include "MoveIterator.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{
 
class MoveIterator_AllBreach : public MoveIterator
{
    std::vector<IsomorphicCardSet>  _isoCardSets;
    std::vector<size_t>             _currentIsoIndex;
    size_t                          _currentFinalBreachIndex;
    size_t                          _movesGenerated;
    size_t                          _legalMovesGenerated;
    size_t                          _currentMove;

    GameState                       _currentState;

    std::vector<Move>               _allMoves;
    Move                            _actionStack;
    size_t                          _finalBreachIndex;
    Action                          _finalBreachAction;
    Action                          _finalBreachActionUndo;
    Card                            _finalBreachCard;
    Action                          _endPhase;

    Move                            _breachAllMove;
    bool                            _canBreachAll;

    bool isTerminalState(size_t isoIndex);
    bool checkSolution();
    void generateAllMoves();
    void recurse(size_t isoIndex);
    void processIsomorphicCards();

public:
    
    MoveIterator_AllBreach(const PlayerID playerID);
    
    void testIterator();

    virtual void            reset();
    virtual void            setState(const GameState & state);

    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription();
    virtual MoveIteratorPtr clone();
};

}