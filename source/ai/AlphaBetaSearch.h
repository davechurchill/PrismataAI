#pragma once

#include "Common.h"
#include "Timer.h"
#include "AlphaBetaSearchParameters.hpp"
#include "AlphaBetaSearchResults.hpp"

namespace Prismata
{

#define PRISMATA_MAX_ALPHABETA_DEPTH 50

class GameState;

class AlphaBetaSearch
{
    static const int ALPHABETA_TIMEOUT = 1;

    AlphaBetaSearchParameters   _params;
    AlphaBetaSearchResults      _results;
    Timer                       _searchTimer;
    size_t                      _currentMaxDepth;

    // we keep a stack of states on the heap so we don't run into stack overflows on deep searches
    std::vector<GameState>      _stateStack;

    MoveIteratorPtr             getMoveIterator(const GameState & state, size_t depth);

public:

 AlphaBetaSearch(const AlphaBetaSearchParameters & params);

    // UCT-specific functions
    AlphaBetaValue              alphaBeta(const GameState & state, size_t depth, StateEvalScore alpha, StateEvalScore beta);
    void                        doSearch(const GameState & initialState, Move & move);
    
    // Move and Child generation functions
    void                        makeMove(GameState & state, const Move & move);

    // Utility functions
    bool                  searchTimeOut();
    bool                  isTerminalState(const GameState & state, const size_t & depth) const;
    AlphaBetaSearchResults &    getResults();
    void                        updateSearchStats(bool forceUpdate = false);
    const StateEvalScore        eval(const GameState & state);

    // graph printing functions
    void                        addToGraphViz(const size_t & depth, const GameState & state, const Move & m, const std::string & moveDesc, StateEvalScore value, const StateEvalScore alpha, const StateEvalScore beta);
    std::string                 getDescription();
};
}