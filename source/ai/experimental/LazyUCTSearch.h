#pragma once

#include "Common.h"
#include "Eval.h"
#include "LazyUCTNode.h"
#include "LazyUCTSearchParameters.hpp"
#include "LazyUCTSearchResults.hpp"
#include "Timer.h"

namespace Prismata
{

class LazyUCTSearch
{
    LazyUCTSearchParameters _params;
    LazyUCTSearchResults    _results;
    Timer                   _searchTimer;
    LazyUCTNode             _rootNode;

    bool                    searchShouldStop();
    bool                    searchTimeOut();
    bool                    isTerminalState(const GameState & state, const size_t depth) const;
    size_t                  allowedChildren(const LazyUCTNode & node) const;
    bool                    shouldGenerateChild(const LazyUCTNode & node) const;
    bool                    generateChild(LazyUCTNode & node, const bool isRoot);
    LazyUCTNode &           selectChild(LazyUCTNode & node);
    LazyUCTNode *           getBestRootNode();
    double                  traverse(LazyUCTNode & node, const size_t depth, const bool isRoot);
    double                  evaluateState(const GameState & state);
    void                    updateResults(bool forceUpdate = false);

public:

    LazyUCTSearch(const LazyUCTSearchParameters & params);

    void                    doSearch(const GameState & initialState, Move & move);
    LazyUCTSearchResults &  getResults();
    std::string             getDescription();
};

}
