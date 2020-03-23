#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"
#include "AlphaBetaSearchSaveState.hpp"
#include <string>

namespace Prismata
{

class AlphaBetaSearchParameters
{
    int         _searchMethod = SearchMethods::IDAlphaBeta;
    PlayerID    _maxPlayer = Players::Player_One;
    int         _maxDepth = 20;

    double      _timeLimit = 0;
    size_t      _maxChildren = 40;
    int         _evalMethod = EvaluationMethods::WillScore;

    bool    _resumeSearch = false;
    AlphaBetaSearchSaveState _saveState;

    PlayerPtr       _playoutPlayers[2];
    MoveIteratorPtr _moveIterators[2];
    MoveIteratorPtr _rootMoveIterators[2];

    //std::string                             _graphVizFilename;  


public:

    // default constructor
    AlphaBetaSearchParameters()
    {

    }

    int searchMethod() const { return _searchMethod; }
    PlayerID maxPlayer() const { return _maxPlayer; }
    int maxDepth() const { return _maxDepth; }
    double timeLimit() const { return _timeLimit; }
    size_t maxChildren() const { return _maxChildren; }
    int evalMethod() const { return _evalMethod; }
    PlayerPtr getPlayoutPlayer(const PlayerID p) const { return _playoutPlayers[p]; }
    bool resumeSearch() const { return _resumeSearch; }
    const AlphaBetaSearchSaveState & getSaveState() const { return _saveState; }
    MoveIteratorPtr & getMoveIterator(const PlayerID p) { return _moveIterators[p]; }
    MoveIteratorPtr & getRootMoveIterator(const PlayerID p) { return _rootMoveIterators[p]; }

    void setSearchMethod(const int & method) { _searchMethod = method; }
    void setMaxPlayer(const PlayerID player) { _maxPlayer = player; }
    void setMaxDepth(const int & depth) { _maxDepth = depth; }
    void setResumeSearch(bool resume, AlphaBetaSearchSaveState ss) { _resumeSearch = resume; _saveState = ss; }
    void setTimeLimit(const double & timeLimit) { _timeLimit = timeLimit; }
    void setMaxChildren(const size_t & children) { _maxChildren = children; }
    void setEvalMethod(const int & eval) { _evalMethod = eval; }
    void setPlayoutPlayer(const PlayerID p, const PlayerPtr & ptr) { _playoutPlayers[p] = ptr; }
    void setMoveIterator(const PlayerID p, const MoveIteratorPtr & m) { _moveIterators[p] = m; }
    void setRootMoveIterator(const PlayerID p, const MoveIteratorPtr & m) { _rootMoveIterators[p] = m; }

    std::string getDescription()
    {
        std::stringstream ss;

        ss << "AB Parameters\n";
        ss << "Time Limit:     ";

        if (timeLimit() > 0)
        {
            ss << timeLimit() << "ms\n";
        }
        else
        {
            ss << "None\n";
        }

        ss << "Max Depth:      "   << (int)maxDepth() << "\n";
        ss << "Max Children:   "   << (int)maxChildren() << "\n";

        return ss.str();
    }
};
}