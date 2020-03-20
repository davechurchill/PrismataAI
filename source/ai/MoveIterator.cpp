#include "MoveIterator.h"

using namespace Prismata;

bool MoveIterator::hasMoreMoves() const
{
    return _hasMoreMoves;
}

bool MoveIterator::generateNextChild(GameState & child, Move & movePerformed)
{
    PRISMATA_ASSERT(false, "Base MoveIterator::getNextState called for some reason");
    return false;
}

void MoveIterator::reset()
{
    PRISMATA_ASSERT(false, "Base MoveIterator::reset called for some reason");
}

void MoveIterator::print()
{
    PRISMATA_ASSERT(false, "Base MoveIterator::reset called for some reason");
}

void MoveIterator::setState(const GameState & state)
{
    PRISMATA_ASSERT(false, "Base MoveIterator::setState called for some reason");
}

void MoveIterator::setBuyLimits(const BuyLimits & buyLimits)
{
    PRISMATA_ASSERT(false, "Base MoveIterator::setBuyLimits called for some reason");
}

void MoveIterator::getRandomMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(false, "Base MoveIterator::getRandomMove called for some reason");
}

PlayerID MoveIterator::getPlayer() const 
{ 
    return _playerID; 
}

MoveIteratorPtr MoveIterator::clone() 
{ 
    return MoveIteratorPtr(new MoveIterator(*this)); 
}

const GameState & MoveIterator::getState() const 
{ 
    return _state; 
}

GameState & MoveIterator::getState() 
{ 
    return _state; 
}