#pragma once

#include "Prismata.h"
#include <map>

namespace Prismata
{
class TestingConfig
{   
    // DEFAULT    DESCRIPTION
    std::vector<GameState>                  _initialStates;
    std::map<std::string, std::string>      _partialPlayerDescriptionMap;
    std::map<std::string, std::string>      _playerDescriptionMap;
    std::map<std::string, std::string>      _moveIteratorDescriptionMap;

    size_t                                  _currentGameIndex = 0;
    std::vector<Game>                       _games;
    
    std::string ReadJsonFile(const std::string & filename);

public:

    TestingConfig();
    
    void parseConfigFile(const std::string & filename);
    void parseGamesJSON(const rapidjson::Value & games, const rapidjson::Value & root);
    
    bool hasMoreGames() const;
    const Game & getNextGame();
    
    static GameState GetAttackTestState();
    static GameState GetSnipeTestState();
    static GameState GetChillTestState();
    static GameState GetFrontlineTestState();
};
}