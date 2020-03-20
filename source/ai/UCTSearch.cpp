#include "UCTSearch.h"
#include "AllPlayers.h"
#include <math.h>
#include "AITools.h"

using namespace Prismata;

UCTSearch::UCTSearch(const UCTSearchParameters & params) 
    : _params(params)
{
}

void UCTSearch::updateResults(bool forceUpdate)
{
    if (forceUpdate || (_results.traversals && (_results.traversals % 200 == 0)))
    {
        _results.timeElapsed = _searchTimer.getElapsedTimeInMilliSec();
        _results.treeSize = _rootNode.memoryUsed() * _results.nodesCreated;

        UCTNode * bestNode = getBestRootNode();
        std::stringstream ss;
        ss << "Possible Moves: " << _rootNode.numChildren() << "\n";
        ss << bestNode->getDescription();
        _results.bestMoveDescription = ss.str();
    }
}

bool UCTSearch::searchShouldStop()
{
    // check search timeout
    if (_results.traversals && (_results.traversals % 10 == 0))
    {
        if (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()))
        {
            return true;
        }
    }

    if (_params.maxTraversals() && (_results.traversals >= _params.maxTraversals()))
    {
        return true;
    }

    return false;
}

void UCTSearch::doSearch(const GameState & initialState, Move & move)
{
    _searchTimer.start();
    _rootNode = UCTNode(NULL, initialState, Players::Player_None, Move(), _params);
    
    // do the traversals
    for (_results.traversals = 0; !searchShouldStop(); ++_results.traversals)
    {
        //GameState state(initialState);
        traverse(_rootNode);//, state);

        updateResults();
    }

    // choose the move to return
    UCTNode * bestNode = getBestRootNode();
    move = bestNode->getMove();

    updateResults(true);
}

UCTNode * UCTSearch::getBestRootNode()
{
    UCTNode * bestNode = NULL;
    if (_params.rootMoveSelectionMethod() == UCTMoveSelect::HighestValue)
    {
        bestNode = &_rootNode.bestUCTValueChild(true, _params);
    }
    else if (_params.rootMoveSelectionMethod() == UCTMoveSelect::MostVisited)
    {
        bestNode = &_rootNode.mostVisitedChild();
    }

    return bestNode;
}

bool UCTSearch::searchTimeOut()
{
    return (_params.timeLimit() && (_searchTimer.getElapsedTimeInMilliSec() >= _params.timeLimit()));
}

const UCTNode & UCTSearch::getRootNode()
{
    return _rootNode;
}

bool UCTSearch::isTerminalState(GameState & state, const size_t & depth) const
{
    return (depth <= 0 || state.isGameOver());
}

UCTNode & UCTSearch::UCTNodeSelect(UCTNode & node)
{
    UCTNode *   bestNode    = nullptr;
    bool        maxPlayer   = node.getChild(0).getPlayerWhoMoved() == _params.maxPlayer();
    double      bestVal     = std::numeric_limits<double>::lowest();
    
    // loop through each child to find the best node
    for (size_t c(0); c < node.numChildren(); ++c)
    {
        UCTNode & child = node.getChild(c);

        // if we have visited this node already, get its UCT value
        if (child.numVisits() > 0)
        {
            double winRate = (double)child.numWins() / (double)child.numVisits();         
            double uctVal = _params.cValue() * sqrt( log( (double)node.numVisits() ) / ( child.numVisits() ) );
            double currentVal = maxPlayer ? (winRate + uctVal) : (1-winRate + uctVal);

            child.setUCTVal(currentVal);

            // choose the best node
            if (currentVal > bestVal)
            {
                bestVal = currentVal;
                bestNode = &child;
            }
        }
        else
        {
            // if we haven't visited it yet, return it and visit immediately
            return child;
        }
    }

    return *bestNode;
}


PlayerID UCTSearch::traverse(UCTNode & node)//, GameState & currentState)
{
    PlayerID stateEval;

    // update the game state with this node's move

    const GameState & currentState = node.getState();

    // if we haven't visited this node yet
    if ((&node != &_rootNode) && (node.numVisits() == 0))
    {
        stateEval = Eval::PerformPlayout(currentState, _params.getPlayoutPlayer(Players::Player_One), _params.getPlayoutPlayer(Players::Player_Two));
        _results.nodesVisited++;
    }
    // otherwise we have seen this node before
    else
    {
        // if the state is terminal
        if (currentState.isGameOver())
        {
            // update the value
            stateEval = currentState.winner();
        }
        else
        {
            // if the children haven't been generated yet
            //if (!node.hasChildren())
            //{
            //    generateChildren(node, currentState);
            //}

            node.generateNextChild(_params);

            UCTNode & next = UCTNodeSelect(node);
            stateEval = traverse(next);//, currentState);
        }
    }

    node.incVisits();
    _results.totalVisits++;

    // if the evaluation syas the current player wins, update the win count
    if (stateEval == _params.maxPlayer())
    {
        node.addWins(1);
    }
    else if (stateEval == Players::Player_None)
    {
        node.addWins(0.5);
    }

    return stateEval;
}

// generate the children of state 'node'
// state is the GameState after node's moves have been performed
void UCTSearch::generateChildren(UCTNode & node, GameState & state)
{
    ////node.generateAllChildren(_params);

    //const PlayerID playerToMove(state.getActivePlayer());

    //MoveIteratorPtr iterCopy = _params.getMoveIterator(playerToMove)->clone();
    //iterCopy->setState(state);
    //Move movePerformed;

    //GameState child;  
    //size_t childNum = 0;
    //while (iterCopy->generateNextChild(child, movePerformed) && (_params.maxChildren() == 0 || childNum < _params.maxChildren()))
    //{
    //    UCTNode child(&node, playerToMove, movePerformed, _params);
    //    node.addChild(child);

    //    _results.nodesCreated++;
    //    childNum++;
    //}
}

bool UCTSearch::isRoot(UCTNode & node) const
{
    return &node == &_rootNode;
}

void UCTSearch::printSubTree(const UCTNode & node, GameState s, std::string filename, size_t maxDepth)
{
    GraphViz::Graph G("g");
    G.set("bgcolor", "#ffffff");

    printSubTreeGraphViz(node, G, s, maxDepth, 0);

    G.printToFile(filename);
}

void UCTSearch::printSubTreeGraphViz(const UCTNode & node, GraphViz::Graph & g, GameState state, size_t maxDepth, size_t depth)
{
    state.doMove(node.getMove());

    std::stringstream label;
    std::stringstream move;
    bool detailed = false;

    //move << node.getMove().toString();
    //move << AITools::GetMoveString(node.getMove(), state);

    if (node.getMove().size() == 0)
    {
        move << "root\n";
    }

    label << node.getUCTVal() << "\n";
    label << "v=" << node.numVisits() << ", w=" << node.numWins() << "\n\n";

    const Move & m = node.getMove();
    for (size_t a(0); a < m.size(); ++a)
    {
        const Action & action = m.getAction(a);

        if (action.getType() == ActionTypes::BUY)
        {
            const CardBuyable & cb = state.getCardBuyableByID(action.getID());
            move << cb.getType().getUIName() << "\n";
        }
    }

    label << move.str();

    if (detailed)
    {
        label << node.getUCTVal() << "\n";
        label << "v=" << node.numVisits() << ", w=" << node.numWins() << "\n\n";
        label << move.str() << "-------------\n";
        label << node.getDescription() << "\n\n";

        if (state.getResources(Players::Player_One).getIntString().size() > 0)
        {
            label << state.getResources(Players::Player_One).getIntString() << "\n";
        }
        label << AITools::GetTypeString(Players::Player_One, state) << "\n";

        if (state.getResources(Players::Player_Two).getIntString().size() > 0)
        {
            label << state.getResources(Players::Player_Two).getIntString() << "\n";
        }
    }

    //label << AITools::GetTypeString(Players::Player_One, state);
    //label << AITools::GetTypeString(Players::Player_Two, state);

    std::string fillcolor       ("#aaaaaa");

    if (node.getPlayerWhoMoved() == Players::Player_One)
    {
        fillcolor = "#ff0000";
    }
    else if (node.getPlayerWhoMoved() == Players::Player_Two)
    {
        fillcolor = "#00ff00";
    }
    
    GraphViz::Node n(getNodeIDString(node));
    n.set("label",      label.str());
    n.set("fillcolor",  fillcolor);
    n.set("color",      "#000000");
    n.set("fontcolor",  "#000000");
    n.set("style",      "filled,bold");
    n.set("shape",      "box");
    g.addNode(n);

    if (depth > maxDepth)
    {
        return;
    }

    // recurse for each child
    for (size_t c(0); c<node.numChildren(); ++c)
    {
        const UCTNode & child = node.getChild(c);
        if (child.numVisits() > 0)
        {
            GraphViz::Edge edge(getNodeIDString(node), getNodeIDString(child));
            g.addEdge(edge);
            printSubTreeGraphViz(child, g, state, maxDepth, depth + 1);
        }
    }
}
 
std::string UCTSearch::getNodeIDString(const UCTNode & node)
{
    std::stringstream ss;
    ss << (unsigned long long)&node;
    return ss.str();
}

UCTSearchResults & UCTSearch::getResults()
{
    return _results;
}

std::string UCTSearch::getDescription()
{
    std::stringstream ss;

    #ifdef PRISMATA_FLASH_VERSION
        ss << _results.traversals << "traversals performed at " << ((double)_results.traversals / _results.timeElapsed * 1000) << " traversals per second!";
    #else
        ss << _params.getDescription();
        ss << _results.getDescription();
    #endif


    return ss.str();
}