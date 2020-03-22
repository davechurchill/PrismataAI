#pragma once

#include <limits>

#include "Timer.h"
#include "GameState.h"
#include "UCTSearchParameters.hpp"
#include "UCTSearchResults.hpp"
#include "UCTNode.h"
#include "UCTMemoryPool.hpp"
#include "Eval.h"
#include "GraphViz.hpp"

namespace Prismata
{

class Game;
class Player;

class UCTSearch
{
    UCTSearchParameters     _params;
    UCTSearchResults        _results;
    Timer                   _searchTimer;
    UCTNode                 _rootNode;

    GameState               _initialState;

public:

    UCTSearch(const UCTSearchParameters & params);

    // UCT-specific functions
    UCTNode &           UCTNodeSelect(UCTNode & parent);
    PlayerID            traverse(UCTNode & node);//, GameState & currentState);
    void                uct(GameState & state, size_t depth, const int lastPlayerToMove);
    UCTNode *           getBestRootNode();
    
    bool                searchShouldStop();
    void                updateResults(bool forceUpdate = false);
    void                doSearch(const GameState & initialState, Move & move);
    
    // Move and Child generation functions
    void                generateChildren(UCTNode & node, GameState & state);
    void                makeMove(UCTNode & node, GameState & state);

    // Utility functions
    const PlayerID      getPlayerToMove(UCTNode & node, const GameState & state) const;
    bool          searchTimeOut();
    bool          isRoot(UCTNode & node) const;
    bool          isTerminalState(GameState & state, const size_t & depth) const;
    void                updateState(UCTNode & node, GameState & state, bool isLeaf);
    void                setMemoryPool(UCTMemoryPool * pool);
    UCTSearchResults &  getResults();
    const UCTNode &     getRootNode();

    // graph printing functions
    void                printSubTree(const UCTNode & node, GameState state, std::string filename, size_t maxDepth);
    void                printSubTreeGraphViz(const UCTNode & node, GraphViz::Graph & g, GameState state, size_t maxDepth, size_t depth);
    std::string         getNodeIDString(const UCTNode & node);

    std::string         getDescription();

};
}