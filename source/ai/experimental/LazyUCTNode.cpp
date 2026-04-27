#include "LazyUCTNode.h"

using namespace Prismata;

LazyUCTNode::LazyUCTNode()
{

}

LazyUCTNode::LazyUCTNode(const GameState & state, const PlayerID playerWhoMoved, const Move & move, const LazyUCTSearchParameters & params, const bool isRoot)
    : _playerWhoMoved(playerWhoMoved)
    , _move(move)
    , _state(state)
{
    if (params.maxChildren() > 0)
    {
        _children.reserve(params.maxChildren());
    }
    else
    {
        _children.reserve(40);
    }

    if (isRoot)
    {
        _moveIterator = params.getRootMoveIterator(state.getActivePlayer())->clone();
    }
    else
    {
        _moveIterator = params.getMoveIterator(state.getActivePlayer())->clone();
    }

    _moveIterator->setState(state);
}

void LazyUCTNode::addVisit(const double score)
{
    _numVisits++;
    _scoreSum += score;
}

bool LazyUCTNode::generateNextChild(const LazyUCTSearchParameters & params, const bool isRoot)
{
    if (!_hasMoreChildren)
    {
        return false;
    }

    if (params.maxChildren() > 0 && _children.size() >= params.maxChildren())
    {
        return false;
    }

    Move movePerformed;
    GameState child;

    if (_moveIterator->generateNextChild(child, movePerformed))
    {
        _children.push_back(LazyUCTNode(child, _state.getActivePlayer(), movePerformed, params, false));
        return true;
    }

    _hasMoreChildren = false;
    return false;
}

LazyUCTNode & LazyUCTNode::mostVisitedChild()
{
    PRISMATA_ASSERT(!_children.empty(), "LazyUCTNode has no children");

    LazyUCTNode * bestChild = &_children.front();
    size_t bestVisits = bestChild->numVisits();
    double bestScore = bestChild->averageScore();

    for (size_t c(1); c < _children.size(); ++c)
    {
        LazyUCTNode & child = _children[c];

        if (child.numVisits() > bestVisits || (child.numVisits() == bestVisits && child.averageScore() > bestScore))
        {
            bestChild = &child;
            bestVisits = child.numVisits();
            bestScore = child.averageScore();
        }
    }

    return *bestChild;
}

LazyUCTNode & LazyUCTNode::highestValueChild()
{
    PRISMATA_ASSERT(!_children.empty(), "LazyUCTNode has no children");

    LazyUCTNode * bestChild = &_children.front();
    double bestScore = bestChild->averageScore();

    for (size_t c(1); c < _children.size(); ++c)
    {
        LazyUCTNode & child = _children[c];

        if (child.averageScore() > bestScore)
        {
            bestChild = &child;
            bestScore = child.averageScore();
        }
    }

    return *bestChild;
}

std::string LazyUCTNode::getDescription() const
{
    std::stringstream ss;

    ss << "Visits=" << numVisits() << " Score=" << averageScore() << " Sum=" << scoreSum() << " Val=" << getUCTVal() << "\n";

    return ss.str();
}
