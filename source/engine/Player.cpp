#include "Player.h"

using namespace Prismata;

void Player::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(false, "Base Player::getMove called for some reason");
}

const int Player::ID() 
{ 
    return m_playerID; 
}

void Player::setID(const int playerID)
{
    m_playerID = playerID;
}

std::string Player::getDescription() 
{ 
    return "Player Type:    ERROR"; 
}

void Player::setDescription(const std::string & desc) 
{ 
    m_description = desc; 
}

PlayerPtr Player::clone() 
{ 
    return PlayerPtr(new Player(*this));
}