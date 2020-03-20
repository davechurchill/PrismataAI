#include "StackAlphaBetaSearch.h"
#include "Eval.h"
#include "MoveIterator_PPPortfolio.h"

using namespace Prismata;

//#define STACKALPHABETA_DEBUG_OUTPUT

StackAlphaBetaSearch::StackAlphaBetaSearch(const AlphaBetaSearchParameters & params) 
 : _params(params)
 , _currentMaxDepth(1)
 , _stack(PRISMATA_MAX_ALPHABETA_DEPTH+10)
 , _depth(0)
 , _resuming(false)
 , _rootNodeMovesCalculated(false)
{
    _results.setMaxDepth(params.maxDepth());
    _params = params;

    if (_params.getMoveIterator(0).get())
    {
        for (size_t i(0); i<PRISMATA_MAX_ALPHABETA_DEPTH; ++i) 
        {
            for (PlayerID p(0); p<2; ++p)
            {
                _stack[i].moveIterator[p] = ((i == 0) ? _params.getRootMoveIterator(p)->clone() : _params.getMoveIterator(p)->clone());
            }
        }
    }
}

void StackAlphaBetaSearch::doSearch(const GameState & initialState, Move & move)
{   
    _results.searchInProgress = true;
    _searchTimer.start();
    unsigned long long lastNodesSearched = 0;

    for (; _currentMaxDepth <= _params.maxDepth(); _currentMaxDepth++)
    {
        unsigned long long previousNodes = _results.nodesExpanded;

        if (!_resuming)
        {
            _depth = 0;
            _stack[_depth].alpha = -1000000;
            _stack[_depth].beta  =  1000000;
            _stack[_depth].state = initialState;
        }
        else
        {
            _resuming = false;
        }

        int returnVal = stackAlphaBeta();
                
        if (returnVal == ALPHABETA_COMPLETE)
        {
            _results.maxDepthCompleted = _currentMaxDepth;
            _results.totalTimeElapsed = _searchTimer.getElapsedTimeInMilliSec();
            
            if ((lastNodesSearched == (_results.nodesExpanded - previousNodes)) || (_currentMaxDepth == _params.maxDepth()))
            {
                _results.searchCompleted = true;
                break;
            }

            lastNodesSearched = _results.nodesExpanded - previousNodes;
            previousNodes = _results.nodesExpanded;
        }
        else if (returnVal == ALPHABETA_TIMEOUT)
        {
            _resuming = true;
            updateResults(true);
            break;
        }
        else
        {
            PRISMATA_ASSERT(false, "Unknown alpha beta return type");
        }
    }

    if (_results.maxDepthCompleted > 0)
    {
        move = _results.bestMoves[_results.maxDepthCompleted];
    }
    else
    {
        PRISMATA_ASSERT(_currentMaxDepth == 1, "This should only happen if the root isn't fully searched");

        move = _results.bestMoves[_currentMaxDepth];
    }


    _results.searchInProgress = false;
}

#define MOVE            _stack[_depth].currentMove
#define ALPHA           _stack[_depth].alpha
#define BETA            _stack[_depth].beta
#define STATE           _stack[_depth].state
#define CHILD_ALPHA     _stack[_depth+1].alpha
#define CHILD_BETA      _stack[_depth+1].beta
#define CHILD_STATE     _stack[_depth+1].state
#define CHILD_NUM       _stack[_depth].variation
#define PLAYER          (STATE.getActivePlayer())
#define MAX_PLAYER      (PLAYER == _params.maxPlayer())
#define MOVE_ITERATOR   _stack[_depth].moveIterator[PLAYER]

#define AB_CALL_RETURN  if (_depth == 0) { return ALPHABETA_COMPLETE; } else { _depth--; goto AB_RETURN; }
#define AB_CALL_RECURSE { _depth++; goto AB_BEGIN; }

int StackAlphaBetaSearch::stackAlphaBeta()
{
    AlphaBetaValue returnVal(0);
    
AB_BEGIN: 

    PRISMATA_ASSERT((size_t)(_depth + 1) < _stack.size(), "Depth is dangerously close to the stack size");

    if (searchTimeOut())
    {
        updateResults(true);
        return ALPHABETA_TIMEOUT;
    }
        
    updateResults();
    
    if (isTerminalState(STATE, _depth))
    {
        returnVal = eval(STATE);
        AB_CALL_RETURN;
    }
    
    CHILD_NUM = 0;
    MOVE_ITERATOR->setState(STATE);
    while (MOVE_ITERATOR->generateNextChild(CHILD_STATE, MOVE))
    {        
        _results.rootNumChildren = (_depth == 0 ? CHILD_NUM + 1 : _results.rootNumChildren);
        _results.rootCurrentChild = (_depth == 0 ? CHILD_NUM : _results.rootCurrentChild);
        
        CHILD_NUM++;
        CHILD_ALPHA = ALPHA;
        CHILD_BETA = BETA;

        AB_CALL_RECURSE;

AB_RETURN:

        if (MAX_PLAYER && (returnVal > ALPHA)) 
        {
            ALPHA = returnVal;
        }
        else if (!MAX_PLAYER && (returnVal <  BETA))
        {
            BETA = returnVal;
        }

        // if we're at the root node, update the move choice
        if (_depth == 0 && (returnVal > _results.bestMoveValues[_currentMaxDepth]))
        {
            _results.bestMoves[_currentMaxDepth]        = MOVE;
            _results.bestMoveValues[_currentMaxDepth]   = returnVal;
        }

        if (ALPHA >= BETA) 
        { 
            break; 
        }
    }
         
    // node is complete, update branching factor information
    _results.nodesCompleted++;
    _results.branches += CHILD_NUM;

    returnVal = MAX_PLAYER ? ALPHA : BETA;
        
    AB_CALL_RETURN;
}

void StackAlphaBetaSearch::updateResults(bool forceUpdate)
{
    _results.nodesExpanded++;
    if (forceUpdate || ((_results.nodesExpanded > 0) && (_results.nodesExpanded % 50 == 0)))
    {
        _results.totalTimeElapsed = _searchTimer.getElapsedTimeInMilliSec();
        _results.currentAlpha = ALPHA;
        _results.currentBeta = BETA;
        _results.totalTimeElapsed = _searchTimer.getElapsedTimeInMilliSec();
    }
}

bool StackAlphaBetaSearch::searchTimeOut()
{
    return (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

bool StackAlphaBetaSearch::isTerminalState(const GameState & state, const size_t & depth) const
{
    return (depth >= _currentMaxDepth || state.isGameOver());
}

const StateEvalScore StackAlphaBetaSearch::eval(const GameState & state)
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

std::string StackAlphaBetaSearch::getDescription()
{
    std::stringstream ss;

    ss << _params.getDescription();
    ss << _results.getDescription(_results.maxDepthCompleted);
    
    return ss.str();
}

AlphaBetaSearchResults & StackAlphaBetaSearch::getResults()
{
    return _results;
}
