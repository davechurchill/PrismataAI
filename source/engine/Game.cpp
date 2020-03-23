#include "Game.h"

using namespace Prismata;

Game::Game(const GameState & initialState, PlayerPtr p1, PlayerPtr p2)
    : m_state(initialState)
{
    m_players[Players::Player_One] = p1;
    m_players[Players::Player_Two] = p2;
}

void Game::play()
{
    PRISMATA_ASSERT(m_players[0].get() != NULL, "Game: Input Player 1 is a null pointer");
    PRISMATA_ASSERT(m_players[1].get() != NULL, "Game: Input Player 2 is a null pointer");

    while(!gameOver())
    {
        playNextTurn();
    }
}

PlayerPtr Game::getPlayerToMove()
{
    return m_players[m_state.getActivePlayer()];
}

void Game::playNextTurn()
{    
    PlayerPtr player = getPlayerToMove();
        
    m_previousMove.clear();
    player->getMove(m_state, m_previousMove);

    PRISMATA_ASSERT(m_previousMove.size() > 0, "Calculated move had size 0, will cause infinite loop");
    
    doMove(m_previousMove);

    m_actions += m_previousMove.size();
    m_turnsPlayed++;
}

const PlayerPtr Game::getPlayer(const PlayerID player) const
{
    return m_players[player];
}

void Game::doMove(const Move & m, bool checkActionLegal)
{
    for (ActionID a(0); a < m.size(); ++a)
    {
        if (!m_state.isLegal(m.getAction(a)))
        {
            std::cout << m.toString();
        }

        bool didAction = doAction(m.getAction(a));

        PRISMATA_ASSERT(didAction, "Tried to do an illegal action");
    }
}

const Move & Game::getPreviousMove() const
{
    return m_previousMove;
}

bool Game::doAction(const Action & action)
{
    return m_state.doAction(action);
}

int Game::getTurnsPlayed()
{
    return m_turnsPlayed;
}

int Game::getActions()
{
    return m_actions;
}

void Game::setTurnLimit(const TurnType limit)
{
    m_turnLimit = limit;
}

bool Game::gameOver() const
{
    return m_state.isGameOver() || (m_turnsPlayed >= m_turnLimit);
}

const GameState & Game::getState() const
{
    return m_state;
}

std::string Game::getWinnerString() const
{
    PlayerID winner = m_state.winner();

    if (m_state.winner() == Players::Player_One) { return "White"; }
    else if (m_state.winner() == Players::Player_Two) { return "Black"; }
    else { return "Draw"; }
}
