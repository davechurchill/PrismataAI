#include "Player_RandomFromIterator.h"

using namespace Prismata;

Player_RandomFromIterator::Player_RandomFromIterator (const PlayerID playerID, const MoveIteratorPtr & moveIterator)
{
    _moveIterator = moveIterator;
    m_playerID = playerID;
}

void Player_RandomFromIterator::getMove(const GameState & state, Move & move)
{
    _moveIterator->getRandomMove(state, move);
}

PlayerPtr Player_RandomFromIterator::clone() 
{ 
    PlayerPtr ret(new Player_RandomFromIterator(m_playerID, _moveIterator->clone()));
    ret->setDescription(m_description);

    return ret;
}

std::string Player_RandomFromIterator::getDescription() 
{ 
    return m_description + "\n" + _moveIterator->getDescription(); 
};