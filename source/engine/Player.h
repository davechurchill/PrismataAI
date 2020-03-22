#pragma once

#include "Common.h"
#include "GameState.h"


namespace Prismata
{

class Player;

typedef std::shared_ptr<Player> PlayerPtr; 
 
class Player 
{

protected:

    PlayerID                m_playerID      = 0;
    std::string             m_description   = "Base Player";

public:

    virtual void            getMove(const GameState & state, Move & move);
    const int               ID();
    void                    setID(const int playerid);
    virtual std::string     getDescription();
    virtual void            setDescription(const std::string & desc);

    virtual PlayerPtr       clone();
};

}