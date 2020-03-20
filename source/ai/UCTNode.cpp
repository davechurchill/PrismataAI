#include "UCTNode.h"

using namespace Prismata;

UCTNode::UCTNode ()
    : _numVisits            (0)
    , _numWins              (0)
    , _uctVal               (0)
    , _playerWhoMoved       (Players::Player_None)
    , _parent               (NULL)
{

}

UCTNode::UCTNode (UCTNode * parent, const GameState & state, const PlayerID playerWhoMoved, const Move & move, const UCTSearchParameters & params, const char * desc)
    : _numVisits            (0)
    , _numWins              (0)
    , _uctVal               (0)
    , _playerWhoMoved       (playerWhoMoved)
    , _move                 (move)
    , _parent               (parent)
    , _description          (desc ? desc : "")
    , _maxChildren          (params.maxChildren())
{
    if (_maxChildren > 0)
    {
        _children.reserve(_maxChildren);
    }
    else
    {
        _children.reserve(20);
    }

    // if it's the root node get the root iterator
    if (_parent == NULL)
    {
        _moveIterator = params.getRootMoveIterator(state.getActivePlayer())->clone();
    }
    else
    {
        _moveIterator = params.getMoveIterator(state.getActivePlayer())->clone();
    }

        
    _moveIterator->setState(state);
}


// attempts to generate the next child of this node, returns true if one was generated
bool UCTNode::generateNextChild(const UCTSearchParameters & params)
{
    if (_children.size() >= _maxChildren)
    {
        return false;
    }

    Move movePerformed;
    GameState child(getState());

    if (_moveIterator->generateNextChild(child, movePerformed))
    {
        _children.push_back(UCTNode(this, child, getState().getActivePlayer(), movePerformed, params));
        return true;
    }
    else
    {
        return false;
    }
}
    
void UCTNode::generateAllChildren(const UCTSearchParameters & params)
{
    while (generateNextChild(params)) {}
}

UCTNode & UCTNode::mostVisitedChild() 
{
    UCTNode * mostVisitedChild = NULL;
    size_t mostVisits = 0;

    for (size_t c(0); c < numChildren(); ++c)
    {
        UCTNode & child = getChild(c);

        if (!mostVisitedChild || (child.numVisits() > mostVisits))
        {
            mostVisitedChild = &child;
            mostVisits = child.numVisits();
        }
    }

    return *mostVisitedChild;
}

UCTNode & UCTNode::bestUCTValueChild(bool maxPlayer, const UCTSearchParameters & params) 
{
    UCTNode * bestChild = NULL;
    double bestVal = maxPlayer ? -1000000 : 1000000;

    for (size_t c(0); c < numChildren(); ++c)
    {
        UCTNode & child = getChild(c);
       
        double winRate      = (double)child.numWins() / (double)child.numVisits();
        double uctVal       = params.cValue() * sqrt( log( (double)numVisits() ) / ( child.numVisits() ) );
        double currentVal   = maxPlayer ? (winRate + uctVal) : (winRate - uctVal);

        if (maxPlayer)
        {
            if (currentVal > bestVal)
            {
                bestVal             = currentVal;
                bestChild           = &child;
            }
        }
        else if (currentVal < bestVal)
        {
            bestVal             = currentVal;
            bestChild           = &child;
        }
    }

    return *bestChild;
}

bool UCTNode::containsChildMove(const Move & move) const
{
    for (size_t c(0); c < numChildren(); ++c)
    {
        const UCTNode & child = getChild(c);

        if (child.getMove() == move)
        {
            return true;
        }
    }

    return false;
}

const size_t UCTNode::numVisits() const 
{
    return _numVisits; 
}

const double UCTNode::numWins() const 
{ 
    return _numWins; 
}

const size_t UCTNode::numChildren() const 
{ 
    return _children.size(); 
}

const double UCTNode::getUCTVal() const 
{ 
    return _uctVal; 
}

bool UCTNode::hasChildren() const 
{ 
    return numChildren() > 0; 
}

const PlayerID UCTNode::getPlayerWhoMoved() const 
{ 
    return _playerWhoMoved; 
}

bool UCTNode::hasChildrenToGenerate() const 
{ 
    return (_children.size() < _maxChildren); 
}

const GameState & UCTNode::getState() const 
{ 
    return _moveIterator->getState(); 
}

std::string UCTNode::getDescription() const           
{ 
    std::stringstream ss;
    ss << "Visits=" << numVisits() <<" Wins=" << numWins() << " Val=" << getUCTVal() << "\n" << _description;
    return ss.str(); 
}

UCTNode * UCTNode::getParent() const 
{ 
    return _parent; 
}

UCTNode & UCTNode::getChild(const size_t & c) 
{ 
    return _children[c]; 
}

const UCTNode & UCTNode::getChild(const size_t & c) const 
{ 
    return _children[c]; 
}

void UCTNode::setUCTVal(double val) 
{ 
    _uctVal = val; 
}

void UCTNode::incVisits() 
{ 
    _numVisits++; 
}

void UCTNode::addWins(double val)
{ 
    _numWins += val; 
}

void UCTNode::setDescription(std::string & desc)
{ 
    _description = desc; 
}

std::vector<UCTNode> & UCTNode::getChildren() 
{ 
    return _children; 
}

const Move & UCTNode::getMove() const
{
    return _move;
}

int UCTNode::memoryUsed()
{
    return sizeof(*this) + (_children.capacity() * sizeof(Move));
}

void UCTNode::setMove(const Move & move)
{
    _move = move;
}
