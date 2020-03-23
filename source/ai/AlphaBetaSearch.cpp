#include "AlphaBetaSearch.h"
#include "Eval.h"

using namespace Prismata;

AlphaBetaSearch::AlphaBetaSearch(const AlphaBetaSearchParameters & params) 
    : _params(params)
    , _currentMaxDepth(1)
    , _stateStack(_params.maxDepth() + 1)
{
    _results.setMaxDepth(params.maxDepth());
}

void AlphaBetaSearch::doSearch(const GameState & initialState, Move & move)
{   
    _searchTimer.start();
    _results.timedOut = false;
        
    unsigned long long lastNodesSearched = 0;

    try
    {
        for (; (int)_currentMaxDepth <= _params.maxDepth(); _currentMaxDepth++)
        {                       
            unsigned long long previousNodes = _results.nodesExpanded;

            AlphaBetaValue alpha = -100000;
            AlphaBetaValue beta  =  100000;
                        
            _results.currentAlpha = alpha;
            _results.currentBeta = beta;
            _results.bestMoveValues[_currentMaxDepth] = alpha;
    
            AlphaBetaValue val = alphaBeta(initialState, 0, alpha, beta);

            _results.maxDepthCompleted = _currentMaxDepth;
            _results.timeElapsed[_currentMaxDepth] = _searchTimer.getElapsedTimeInMilliSec();
            
            if ((lastNodesSearched == (_results.nodesExpanded - previousNodes)) || (_currentMaxDepth == _params.maxDepth()))
            {
                _results.searchCompleted = true;
                break;
            }

            lastNodesSearched = _results.nodesExpanded - previousNodes;
            previousNodes = _results.nodesExpanded;
        }
    }
    catch (int e)
    {
        if (e == ALPHABETA_TIMEOUT)
        {
            _results.timedOut = true;
        }
    }

    move = _results.bestMoves[_results.maxDepthCompleted];
}

std::string AlphaBetaSearch::getDescription()
{
    std::stringstream ss;

    ss << _params.getDescription();
    ss << _results.getDescription(_results.maxDepthCompleted);
    
    return ss.str();
}

AlphaBetaValue AlphaBetaSearch::alphaBeta(const GameState & state, size_t depth, StateEvalScore alpha, StateEvalScore beta)
{
    bool maxPlayer = (state.getActivePlayer() == _params.maxPlayer());

    if (searchTimeOut())
    {
        updateSearchStats(true);
        throw ALPHABETA_TIMEOUT;
    }

    updateSearchStats();

    if (isTerminalState(state, depth))
    {
        return eval(state);
    }
    
    const PlayerID playerToMove(state.getActivePlayer());

    MoveIteratorPtr iter = getMoveIterator(state, depth);
    Move movePerformed;

    GameState & child = _stateStack[depth];
    while (iter->generateNextChild(child, movePerformed))
    {
        AlphaBetaValue value = alphaBeta(child, depth+1, alpha, beta);

        // set alpha or beta based on maxplayer
        if (maxPlayer && (value > alpha)) 
        {
            alpha = value;
        }
        else if (!maxPlayer && (value < beta))
        {
            beta = value;
        }
        
        // if we're at the root node, update the move choice
        if (depth == 0 && (value > _results.bestMoveValues[_currentMaxDepth]))
        {
            _results.bestMoves[_currentMaxDepth]         = movePerformed;
            _results.bestMoveValues[_currentMaxDepth]    = value;
        }

        // alpha-beta cut
        if (alpha >= beta) 
        { 
            break; 
        }
    }
 
    return maxPlayer ? alpha : beta;
}

MoveIteratorPtr AlphaBetaSearch::getMoveIterator(const GameState & state, size_t depth)
{
    MoveIteratorPtr mip; 
    
    if (depth == 0)
    {
        mip = _params.getRootMoveIterator(state.getActivePlayer())->clone();
    }
    else
    {
        mip = _params.getMoveIterator(state.getActivePlayer())->clone();
    }

    mip->setState(state);
    return mip;
}

bool AlphaBetaSearch::searchTimeOut()
{
 return (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

bool AlphaBetaSearch::isTerminalState(const GameState & state, const size_t & depth) const
{
    return (depth >= _currentMaxDepth || state.isGameOver());
}

void AlphaBetaSearch::updateSearchStats(bool forceUpdate)
{
    _results.nodesExpanded++;
    if (forceUpdate || ((_results.nodesExpanded > 0) && (_results.nodesExpanded % 50 == 0)))
    {
        _results.totalTimeElapsed = _searchTimer.getElapsedTimeInMilliSec();
        _results.totalTimeElapsed = _searchTimer.getElapsedTimeInMilliSec();
    }
}

const StateEvalScore AlphaBetaSearch::eval(const GameState & state)
{
    _results.evaluationsPerformed++;

    if (_params.evalMethod() == EvaluationMethods::Playout)
    {
        return Eval::ABPlayoutScore( state, 
                                     _params.getPlayoutPlayer(Players::Player_One), 
                                     _params.getPlayoutPlayer(Players::Player_Two), 
                                     _params.maxPlayer());
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScore)
    {
        return Eval::WillScoreEvaluation(state, _params.maxPlayer());
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScoreInflation)
    {
        return Eval::WillScoreInflationEvaluation(state, _params.maxPlayer());
    }
    else
    {
        PRISMATA_ASSERT(false, "Unknown Evaluation Type");
        return 0;
    }
}

AlphaBetaSearchResults & AlphaBetaSearch::getResults()
{
    return _results;
}
