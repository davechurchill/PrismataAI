#pragma once

#include "Prismata.h"
#include "rapidjson/document.h"

namespace Prismata
{
 
class TournamentGame
{
    Game            _game;
    std::string     _playerNames[2];
    size_t          _playerTotalTimeMS[2];
    size_t          _maxTimeMS[2];
        
public:

    TournamentGame(GameState & initialState, const std::string & p1name, PlayerPtr p1, const std::string & p2name, const PlayerPtr p2);

    void playGame();

    const std::string & getPlayerName(const PlayerID & player) const;
    const GameState & getFinalGameState() const;
    const size_t getTotalTimeMS(const PlayerID & player) const;
    const size_t getMaxTimeMS(const PlayerID & player) const;
};

}