#pragma once

#include "Common.h"
#include "MoveIterator.h"
#include "GameState.h"
#include <math.h>
#include "UCTSearchParameters.hpp"

namespace Prismata
{


class UCTNode
{
    size_t                      _numVisits;         // total visits to this node
    double                      _numWins;           // wins from this node
    double                      _uctVal;            // previous computed UCT value
            
    PlayerID                    _playerWhoMoved;    // the player who made a move to generate this node
    Move                        _move;              // the move that generated this node

    MoveIteratorPtr             _moveIterator;
    size_t                      _maxChildren;

    std::vector<UCTNode>        _children;

    UCTNode *                   _parent;
    std::string                 _description;
    
public:

    UCTNode ();
    UCTNode (UCTNode * parent, const GameState & state, const PlayerID playerWhoMoved, const Move & move, const UCTSearchParameters & params, const char * desc = NULL);

    const size_t            numVisits()                 const;
    const size_t            numChildren()               const;
    const double            numWins()                   const;
    const double            getUCTVal()                 const;
    bool              hasChildren()               const;
    bool              hasChildrenToGenerate()     const;
    const PlayerID          getPlayerWhoMoved()         const;
    const GameState &       getState()                  const;
    std::vector<UCTNode> &  getChildren();

    std::string             getDescription()            const;

    UCTNode *               getParent()                 const;
    UCTNode &               getChild(const size_t & c);
    const UCTNode &         getChild(const size_t & c)  const;
    UCTNode &               mostVisitedChild();
    UCTNode &               bestUCTValueChild(bool maxPlayer, const UCTSearchParameters & params);

    void                    setUCTVal(double val);
    void                    incVisits();
    void                    addWins(double val);
    void                    setDescription(std::string & desc);

    

    const Move & getMove() const;

    int memoryUsed();

    void setMove(const Move & move);
    bool generateNextChild(const UCTSearchParameters & params);
    
    void generateAllChildren(const UCTSearchParameters & params);

    
    bool containsChildMove(const Move & move) const;
};
}