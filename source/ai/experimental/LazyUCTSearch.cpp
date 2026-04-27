#include "LazyUCTSearch.h"

#include <cmath>

using namespace Prismata;

LazyUCTSearch::LazyUCTSearch(const LazyUCTSearchParameters & params)
    : _params(params)
{

}

bool LazyUCTSearch::searchTimeOut()
{
    return (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

bool LazyUCTSearch::searchShouldStop()
{
    if (_results.traversals && (_results.traversals % 10 == 0) && searchTimeOut())
    {
        return true;
    }

    if (_params.maxTraversals() && (_results.traversals >= _params.maxTraversals()))
    {
        return true;
    }

    return false;
}

bool LazyUCTSearch::isTerminalState(const GameState & state, const size_t depth) const
{
    return state.isGameOver() || (_params.maxDepth() > 0 && depth >= _params.maxDepth());
}

size_t LazyUCTSearch::allowedChildren(const LazyUCTNode & node) const
{
    if (!_params.progressiveWidening())
    {
        return _params.maxChildren();
    }

    const double visits = (double)std::max((size_t)1, node.numVisits());
    size_t allowed = (size_t)std::floor(_params.widenC() * std::pow(visits, _params.widenAlpha()));
    allowed = std::max((size_t)1, allowed);

    if (_params.maxChildren() > 0)
    {
        allowed = std::min(allowed, _params.maxChildren());
    }

    return allowed;
}

bool LazyUCTSearch::shouldGenerateChild(const LazyUCTNode & node) const
{
    if (!node.hasMoreChildren())
    {
        return false;
    }

    const size_t allowed = allowedChildren(node);
    return allowed == 0 || node.numChildren() < allowed;
}

bool LazyUCTSearch::generateChild(LazyUCTNode & node, const bool isRoot)
{
    if (!shouldGenerateChild(node))
    {
        return false;
    }

    if (node.generateNextChild(_params, isRoot))
    {
        _results.nodesCreated++;
        return true;
    }

    return false;
}

LazyUCTNode & LazyUCTSearch::selectChild(LazyUCTNode & node)
{
    PRISMATA_ASSERT(node.numChildren() > 0, "LazyUCT selection called with no children");

    LazyUCTNode * bestNode = nullptr;
    double bestVal = std::numeric_limits<double>::lowest();
    const bool maxPlayerToMove = node.getState().getActivePlayer() == _params.maxPlayer();
    const double logVisits = std::log((double)std::max((size_t)2, node.numVisits()));

    for (size_t c(0); c < node.numChildren(); ++c)
    {
        LazyUCTNode & child = node.getChild(c);

        if (child.numVisits() == 0)
        {
            return child;
        }

        const double mean = child.averageScore();
        const double explore = _params.cValue() * std::sqrt(logVisits / (double)child.numVisits());
        const double currentVal = (maxPlayerToMove ? mean : -mean) + explore;

        child.setUCTVal(currentVal);

        if (currentVal > bestVal)
        {
            bestVal = currentVal;
            bestNode = &child;
        }
    }

    return *bestNode;
}

double LazyUCTSearch::evaluateState(const GameState & state)
{
    _results.evaluations++;

    if (state.isGameOver())
    {
        if (state.winner() == _params.maxPlayer())
        {
            return 1.0;
        }
        else if (state.winner() == state.getEnemy(_params.maxPlayer()))
        {
            return -1.0;
        }

        return 0.0;
    }

    if (_params.evalMethod() == EvaluationMethods::Playout)
    {
        PRISMATA_ASSERT(_params.getPlayoutPlayer(Players::Player_One).get() != NULL, "LazyUCT has no Player One playout player");
        PRISMATA_ASSERT(_params.getPlayoutPlayer(Players::Player_Two).get() != NULL, "LazyUCT has no Player Two playout player");

        PlayerID winner = Eval::PerformPlayout(state,
                                               _params.getPlayoutPlayer(Players::Player_One),
                                               _params.getPlayoutPlayer(Players::Player_Two));

        if (winner == _params.maxPlayer())
        {
            return 1.0;
        }
        else if (winner == state.getEnemy(_params.maxPlayer()))
        {
            return -1.0;
        }

        return 0.0;
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScore)
    {
        return Eval::WillScoreEvaluation(state, _params.maxPlayer());
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScoreInflation)
    {
        return Eval::WillScoreInflationEvaluation(state, _params.maxPlayer());
    }

    PRISMATA_ASSERT(false, "Unknown LazyUCT eval method");
    return 0.0;
}

double LazyUCTSearch::traverse(LazyUCTNode & node, const size_t depth, const bool isRoot)
{
    _results.traverseCalls++;
    _results.maxDepthReached = std::max(_results.maxDepthReached, depth);

    double score = 0.0;

    if (!isRoot && node.numVisits() == 0)
    {
        _results.nodesVisited++;
        score = evaluateState(node.getState());
    }
    else if (isTerminalState(node.getState(), depth))
    {
        score = evaluateState(node.getState());
    }
    else
    {
        generateChild(node, isRoot);

        if (node.numChildren() == 0)
        {
            score = evaluateState(node.getState());
        }
        else
        {
            LazyUCTNode & next = selectChild(node);
            score = traverse(next, depth + 1, false);
        }
    }

    node.addVisit(score);
    _results.totalVisits++;

    return score;
}

LazyUCTNode * LazyUCTSearch::getBestRootNode()
{
    if (_rootNode.numChildren() == 0)
    {
        return nullptr;
    }

    if (_params.rootMoveSelectionMethod() == LazyUCTMoveSelect::HighestValue)
    {
        return &_rootNode.highestValueChild();
    }

    return &_rootNode.mostVisitedChild();
}

void LazyUCTSearch::updateResults(bool forceUpdate)
{
    if (forceUpdate || (_results.traversals && (_results.traversals % 200 == 0)))
    {
        _results.timeElapsed = _searchTimer.getElapsedTimeInMilliSec();
        _results.rootChildren = _rootNode.numChildren();

        LazyUCTNode * bestNode = getBestRootNode();
        if (bestNode)
        {
            _results.bestMoveValue = bestNode->averageScore();

            std::stringstream ss;
            ss << "Possible Moves: " << _rootNode.numChildren() << "\n";
            ss << bestNode->getDescription();
            _results.bestMoveDescription = ss.str();
        }
    }
}

void LazyUCTSearch::doSearch(const GameState & initialState, Move & move)
{
    _searchTimer.start();
    _rootNode = LazyUCTNode(initialState, Players::Player_None, Move(), _params, true);

    for (_results.traversals = 0; !searchShouldStop(); ++_results.traversals)
    {
        traverse(_rootNode, 0, true);
        updateResults();
    }

    LazyUCTNode * bestNode = getBestRootNode();
    if (bestNode)
    {
        move = bestNode->getMove();
    }
    else
    {
        MoveIteratorPtr iterator = _params.getRootMoveIterator(initialState.getActivePlayer())->clone();
        GameState child;
        iterator->setState(initialState);

        PRISMATA_ASSERT(iterator->generateNextChild(child, move), "LazyUCT failed to generate any root moves");
    }

    updateResults(true);
}

LazyUCTSearchResults & LazyUCTSearch::getResults()
{
    return _results;
}

std::string LazyUCTSearch::getDescription()
{
    std::stringstream ss;

    ss << _params.getDescription();
    ss << _results.getDescription();

    return ss.str();
}
