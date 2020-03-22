#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"
#include <sstream>

namespace Prismata
{
    class UCTSearchParameters;

    namespace UCTMoveSelect
    {
        enum { HighestValue, MostVisited };
    }
}

class Prismata::UCTSearchParameters
{                   
    PlayerID        _maxPlayer          = Players::Player_One;            
    int        _rootMoveSelection  = UCTMoveSelect::MostVisited;     

    size_t          _timeLimit          = 0;            
    double          _cValue             = 2.0;                
    size_t          _maxTraversals      = 100;         
    size_t          _maxChildren        = 10;           
    int        _evalMethod         = EvaluationMethods::Playout;

    PlayerPtr       _playoutPlayers[2];
    MoveIteratorPtr _moveIterators[2];
    MoveIteratorPtr _rootMoveIterators[2];

    std::string                             _graphVizFilename;      
    
public:

    UCTSearchParameters() 
    {

    }

    const PlayerID maxPlayer()                                    const   { return _maxPlayer; }
    const int & evalMethod()                                   const   { return _evalMethod; }
    const size_t & timeLimit()                                      const   { return _timeLimit; }
    const double & cValue()                                         const   { return _cValue; }
    const size_t & maxTraversals()                                  const   { return _maxTraversals; }
    const size_t & maxChildren()                                    const   { return _maxChildren; }
    const int & rootMoveSelectionMethod()                      const   { return _rootMoveSelection; }
    const std::string & graphVizFilename()                          const   { return _graphVizFilename; }
    const PlayerPtr & getPlayoutPlayer(const PlayerID p)            const   { return _playoutPlayers[p]; }
    const MoveIteratorPtr & getMoveIterator(const PlayerID p)       const   { return _moveIterators[p]; }
    const MoveIteratorPtr & getRootMoveIterator(const PlayerID p)   const   { return _rootMoveIterators[p]; }
 
    void setMaxPlayer(const PlayerID player)                              { _maxPlayer = player; }
    void setEvalMethod(const int & method)                             { _evalMethod = method; }
    void setTimeLimit(const size_t & timeLimit)                             { _timeLimit = timeLimit; }  
    void setCValue(const double & c)                                        { _cValue = c; }
    void setMaxTraversals(const size_t & traversals)                        { _maxTraversals = traversals; }
    void setMaxChildren(const size_t & children)                            { _maxChildren = children; }
    void setRootMoveSelectionMethod(const int & method)                { _rootMoveSelection = method; }
    void setGraphVizFilename(const std::string & filename)                  { _graphVizFilename = filename; }
    void setPlayoutPlayer(const PlayerID p, const PlayerPtr & ptr)          { _playoutPlayers[p] = ptr; }
    void setMoveIterator(const PlayerID p, const MoveIteratorPtr & m)       { _moveIterators[p] = m; }
    void setRootMoveIterator(const PlayerID p, const MoveIteratorPtr & m)   { _rootMoveIterators[p] = m; }

    std::string getDescription()
    {
        std::stringstream ss;

        ss << "UCT Parameters\n";
        ss << "Time Limit:     ";
        
        if (timeLimit() > 0)
        {
            ss << timeLimit() << "ms\n";
        }
        else
        {
            ss << "None\n";
        }
        
        ss << "Max Traversals: "   << maxTraversals() << "\n";
        ss << "Max Children:   "   << maxChildren() << "\n";
        ss << "C Value:        "   << cValue() << "\n";
                
        return ss.str();
    }
};
