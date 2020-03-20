#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"

namespace Prismata
{
 
class MoveIterator;

typedef std::shared_ptr<MoveIterator> MoveIteratorPtr; 

class MoveIterator 
{

protected:

    PlayerID                _playerID;
    bool                    _hasMoreMoves;
    bool                    _firstMove;

    GameState               _state;

public:
    
    virtual bool            hasMoreMoves() const;
    virtual bool            generateNextChild(GameState & child, Move & movePerformed);
    virtual std::string     getDescription() { return "MoveIterator BaseClass"; }
    virtual void            reset();
    virtual void            print();
    virtual void            setState(const GameState & state);
    virtual void            setBuyLimits(const BuyLimits & buyLimits);
    virtual void            getRandomMove(const GameState & state, Move & move);
    virtual PlayerID        getPlayer() const;
    virtual MoveIteratorPtr clone();

    virtual const GameState & getState() const;
    virtual GameState & getState();
};


}