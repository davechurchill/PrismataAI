#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"
#include <sstream>

namespace Prismata
{
    class LazyUCTSearchParameters;

    namespace LazyUCTMoveSelect
    {
        enum { HighestValue, MostVisited };
    }
}

class Prismata::LazyUCTSearchParameters
{
    PlayerID        _maxPlayer = Players::Player_One;
    int             _rootMoveSelection = LazyUCTMoveSelect::MostVisited;

    size_t          _timeLimit = 0;
    size_t          _maxTraversals = 0;
    size_t          _maxChildren = 40;
    size_t          _maxDepth = 20;
    double          _cValue = 1.4;
    bool            _progressiveWidening = false;
    double          _widenC = 2.0;
    double          _widenAlpha = 0.5;
    int             _evalMethod = EvaluationMethods::Playout;

    PlayerPtr       _playoutPlayers[2];
    MoveIteratorPtr _moveIterators[2];
    MoveIteratorPtr _rootMoveIterators[2];

public:

    LazyUCTSearchParameters clone() const
    {
        LazyUCTSearchParameters params(*this);

        for (PlayerID p(0); p < 2; ++p)
        {
            params._playoutPlayers[p] = _playoutPlayers[p].get() ? _playoutPlayers[p]->clone() : PlayerPtr();
            params._moveIterators[p] = _moveIterators[p].get() ? _moveIterators[p]->clone() : MoveIteratorPtr();
            params._rootMoveIterators[p] = _rootMoveIterators[p].get() ? _rootMoveIterators[p]->clone() : MoveIteratorPtr();
        }

        return params;
    }

    const PlayerID maxPlayer() const { return _maxPlayer; }
    const int & evalMethod() const { return _evalMethod; }
    const size_t & timeLimit() const { return _timeLimit; }
    const size_t & maxTraversals() const { return _maxTraversals; }
    const size_t & maxChildren() const { return _maxChildren; }
    const size_t & maxDepth() const { return _maxDepth; }
    const double & cValue() const { return _cValue; }
    const bool & progressiveWidening() const { return _progressiveWidening; }
    const double & widenC() const { return _widenC; }
    const double & widenAlpha() const { return _widenAlpha; }
    const int & rootMoveSelectionMethod() const { return _rootMoveSelection; }
    const PlayerPtr & getPlayoutPlayer(const PlayerID p) const { return _playoutPlayers[p]; }
    const MoveIteratorPtr & getMoveIterator(const PlayerID p) const { return _moveIterators[p]; }
    const MoveIteratorPtr & getRootMoveIterator(const PlayerID p) const { return _rootMoveIterators[p]; }

    void setMaxPlayer(const PlayerID player) { _maxPlayer = player; }
    void setEvalMethod(const int & method) { _evalMethod = method; }
    void setTimeLimit(const size_t & timeLimit) { _timeLimit = timeLimit; }
    void setMaxTraversals(const size_t & traversals) { _maxTraversals = traversals; }
    void setMaxChildren(const size_t & children) { _maxChildren = children; }
    void setMaxDepth(const size_t & depth) { _maxDepth = depth; }
    void setCValue(const double & c) { _cValue = c; }
    void setProgressiveWidening(const bool progressiveWidening) { _progressiveWidening = progressiveWidening; }
    void setWidenC(const double & widenC) { _widenC = widenC; }
    void setWidenAlpha(const double & widenAlpha) { _widenAlpha = widenAlpha; }
    void setRootMoveSelectionMethod(const int & method) { _rootMoveSelection = method; }
    void setPlayoutPlayer(const PlayerID p, const PlayerPtr & ptr) { _playoutPlayers[p] = ptr; }
    void setMoveIterator(const PlayerID p, const MoveIteratorPtr & m) { _moveIterators[p] = m; }
    void setRootMoveIterator(const PlayerID p, const MoveIteratorPtr & m) { _rootMoveIterators[p] = m; }

    std::string getDescription() const
    {
        std::stringstream ss;

        ss << "Lazy UCT Parameters\n";
        ss << "Time Limit:     ";

        if (timeLimit() > 0)
        {
            ss << timeLimit() << "ms\n";
        }
        else
        {
            ss << "None\n";
        }

        ss << "Max Traversals: " << maxTraversals() << "\n";
        ss << "Max Children:   " << maxChildren() << "\n";
        ss << "Max Depth:      " << maxDepth() << "\n";
        ss << "C Value:        " << cValue() << "\n";
        ss << "Progressive:    " << (progressiveWidening() ? "true" : "false") << "\n";

        if (progressiveWidening())
        {
            ss << "Widen C:        " << widenC() << "\n";
            ss << "Widen Alpha:    " << widenAlpha() << "\n";
        }

        return ss.str();
    }
};
