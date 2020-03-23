#pragma once

#include "Common.h"
#include "GameState.h"
#include "Player.h"

namespace Prismata
{
 
class Game
{    
    
    GameState   m_state;
    PlayerPtr   m_players[2];
    int         m_turnsPlayed    = 0;
    int         m_actions        = 0;
    TurnType    m_turnLimit      = 200;
    Move        m_previousMove;

public:
    
    Game(const GameState & initialState, PlayerPtr p1, PlayerPtr p2);

    void                play();
    void                playNextTurn();
    void                doMove(const Move & m, bool checkActionLegal = false);
    void                setTurnLimit(const TurnType limit);
    bool                doAction(const Action & action);
    bool                gameOver() const;
    int                 getTurnsPlayed();
    int                 getActions();
    PlayerPtr           getPlayerToMove();
    const GameState &   getState() const;
    const Move &        getPreviousMove() const;
    std::string         getWinnerString() const;
    const PlayerPtr     getPlayer(const PlayerID player) const;
};


}