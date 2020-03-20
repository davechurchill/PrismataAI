#pragma once

#include "Common.h"
#include "Timer.h"
#include "GameState.h"
#include "AlphaBetaSearchParameters.hpp"
#include "AlphaBetaSearchResults.hpp"

namespace Prismata
{

#define PRISMATA_MAX_ALPHABETA_DEPTH 50

class StackData
{
public:
    size_t              variation;
    StateEvalScore      alpha;
    StateEvalScore      beta;
    GameState           state;
    std::vector<Move>   allMoves;
    Move                currentMove;
    MoveIteratorPtr     moveIterator[2];
    
    StackData()
        : variation(0)
        , alpha(0)
        , beta(0)
    {
    
    }

};

class StackAlphaBetaSearch
{
    static const int ALPHABETA_TIMEOUT = 1;
    static const int ALPHABETA_COMPLETE = 0;

    int                         _depth;
    bool                        _resuming;
    bool                        _rootNodeMovesCalculated;
    std::vector<StackData>      _stack;

    AlphaBetaSearchParameters   _params;
    AlphaBetaSearchResults      _results;
    Timer                       _searchTimer;
    size_t                      _currentMaxDepth;
    std::vector<Move>           _rootNodeMoves;

public:

    StackAlphaBetaSearch(const AlphaBetaSearchParameters & params);

    // search-specific functions
    int                         stackAlphaBeta();
    void                        doSearch(const GameState & initialState, Move & move);
    
    // Move and Child generation functions
    void                        makeMove(GameState & state, const Move & move);

    // Utility functions
    bool                  searchTimeOut();
    bool                  isTerminalState(const GameState & state, const size_t & depth) const;
    AlphaBetaSearchResults &    getResults();
    const StateEvalScore        eval(const GameState & state);
    void                        updateResults(bool forceUpdate = false);

    // graph printing functions
    std::string                 getDescription();
};
}