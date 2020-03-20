#include "TournamentGame.h"
#include "Timer.h"

using namespace Prismata;

TournamentGame::TournamentGame(GameState & initialState, const std::string & p1name, PlayerPtr p1, const std::string & p2name, const PlayerPtr p2)
    : _game(initialState, p1, p2)
{
    _playerNames[0] = p1name;
    _playerNames[1] = p2name;
    _playerTotalTimeMS[0] = 0;
    _playerTotalTimeMS[1] = 0;
    _maxTimeMS[0] = 0;
    _maxTimeMS[1] = 0;
}

void TournamentGame::playGame()
{
    Timer t;
    while(!_game.gameOver())
    {
        PlayerID playerToMove = _game.getState().getActivePlayer();   

        t.start();
        _game.playNextTurn();
        double ms = t.getElapsedTimeInMilliSec();
        _playerTotalTimeMS[playerToMove] += ms;
        _maxTimeMS[playerToMove] = std::max((size_t)ms, _maxTimeMS[playerToMove]);
    }
}

const std::string & TournamentGame::getPlayerName(const PlayerID & player) const
{
    return _playerNames[player];
}

const GameState & TournamentGame::getFinalGameState() const
{
    return _game.getState();
}

const size_t TournamentGame::getTotalTimeMS(const PlayerID & player) const
{
    return _playerTotalTimeMS[player];
}

const size_t TournamentGame::getMaxTimeMS(const PlayerID & player) const
{
    return _maxTimeMS[player];
}