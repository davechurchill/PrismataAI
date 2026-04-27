#pragma once

#include "Common.h"
#include "GameState.h"
#include "LazyUCTSearchParameters.hpp"
#include "MoveIterator.h"

namespace Prismata
{

class LazyUCTNode
{
    size_t                  _numVisits = 0;
    double                  _scoreSum = 0;
    double                  _uctVal = 0;

    PlayerID                _playerWhoMoved = Players::Player_None;
    Move                    _move;
    GameState               _state;
    MoveIteratorPtr         _moveIterator;
    bool                    _hasMoreChildren = true;

    std::vector<LazyUCTNode> _children;

public:

    LazyUCTNode();
    LazyUCTNode(const GameState & state, const PlayerID playerWhoMoved, const Move & move, const LazyUCTSearchParameters & params, const bool isRoot);

    const size_t numVisits() const { return _numVisits; }
    const size_t numChildren() const { return _children.size(); }
    const double scoreSum() const { return _scoreSum; }
    const double averageScore() const { return _numVisits > 0 ? _scoreSum / _numVisits : 0; }
    const double getUCTVal() const { return _uctVal; }
    const PlayerID getPlayerWhoMoved() const { return _playerWhoMoved; }
    const GameState & getState() const { return _state; }
    const Move & getMove() const { return _move; }
    bool hasMoreChildren() const { return _hasMoreChildren; }
    std::vector<LazyUCTNode> & getChildren() { return _children; }

    LazyUCTNode & getChild(const size_t c) { return _children[c]; }
    const LazyUCTNode & getChild(const size_t c) const { return _children[c]; }

    void setUCTVal(const double val) { _uctVal = val; }
    void addVisit(const double score);
    bool generateNextChild(const LazyUCTSearchParameters & params, const bool isRoot);
    LazyUCTNode & mostVisitedChild();
    LazyUCTNode & highestValueChild();
    std::string getDescription() const;
};

}
