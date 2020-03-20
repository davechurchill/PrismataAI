#include "TestingConfig.h"
#include "PrismataAI.h"

using namespace Prismata;

TestingConfig::TestingConfig()
{

}

void TestingConfig::parseConfigFile(const std::string & filename)
{
    rapidjson::Document document;

    std::string json = FileUtils::ReadFile(filename);
    bool parsingFailed = document.Parse(json.c_str()).HasParseError();

    if (parsingFailed)
    {
        int errorPos = document.GetErrorOffset();

        std::stringstream ss;
        ss << std::endl << "JSON Parse Error: " << document.GetParseError() << std::endl;
        ss << "Error Position:   " << errorPos << std::endl;
        ss << "Error Substring:  " << json.substr(errorPos-5, 10) << std::endl;

        PRISMATA_ASSERT(!parsingFailed, "Error parsing JSON config file: %s", ss.str().c_str());
    }

    PRISMATA_ASSERT(document.HasMember("States"),           "No 'States' Found");
    PRISMATA_ASSERT(document.HasMember("Games"),            "No 'Games' Options Found");

    const rapidjson::Value & guiValue = document["GUI"];
    PRISMATA_ASSERT(guiValue.HasMember("Enabled"), "GUI has no 'Enabled' option");
    PRISMATA_ASSERT(guiValue.HasMember("Width"), "GUI has no 'Width' option");
    PRISMATA_ASSERT(guiValue.HasMember("Height"), "GUI has no 'Height' option");
    PRISMATA_ASSERT(guiValue.HasMember("ActionDelay"), "GUI has no 'ActionDelay' option");
    PRISMATA_ASSERT(guiValue.HasMember("TurnDelay"), "GUI has no 'TurnDelay' option");

    parseGamesJSON(document["Games"], document);

    printf("Parsing of config file complete\n");
}

bool TestingConfig::hasMoreGames() const
{
    return _currentGameIndex < _games.size();
}

const Game & TestingConfig::getNextGame()
{
    PRISMATA_ASSERT(hasMoreGames(), "No more games yo");

    return _games[_currentGameIndex++];
}

void TestingConfig::parseGamesJSON(const rapidjson::Value & games, const rapidjson::Value & root)
{
    Timer t;
    t.start();
    for (size_t gg(0); gg<games.Size(); ++gg)
    {
        const rapidjson::Value & game = games[gg];

        // if we don't want to play this set of games, just skip it
        if (game.HasMember("play") && game["play"].IsBool() && !game["play"].GetBool())
        {
            continue;
        }

        PRISMATA_ASSERT(game.HasMember("name"), "Game has no 'name' option");
        PRISMATA_ASSERT(game.HasMember("state") && game["state"].IsString(), "Game has no 'state' String option");
        PRISMATA_ASSERT(game.HasMember("WhitePlayer") && game["WhitePlayer"].IsString(), "Game has no 'WhitePlayer' String option");
        PRISMATA_ASSERT(game.HasMember("BlackPlayer") && game["BlackPlayer"].IsString(), "Game has no 'BlackPlayer' String option");
        size_t numGames = game.HasMember("games") ? game["games"].GetInt() : 1;

        std::vector<int> winners(3,0);
        for (size_t i(0); i < numGames; ++i)
        {
            GameState state = AIParameters::Instance().getState(game["state"].GetString());

            for (int x=0; x < 10; ++x)
            {
                //state = GetStateFromVariable(game["state"].GetString(), root);
            }

            PlayerPtr white = AIParameters::Instance().getPlayer(Players::Player_One, game["WhitePlayer"].GetString());
            PlayerPtr black = AIParameters::Instance().getPlayer(Players::Player_Two, game["BlackPlayer"].GetString());


            Game g(state, white, black);

            g.play();
                
            if (i % 500 == 0)
            {
                double ms = t.getElapsedTimeInMilliSec();
                double gps = (i * 1000.0) / ms;
                std::cout << "Played game " << i << ", time = " << ms/1000.0 << " @ " << gps << " games per second \n";
            }
        }
    }
}

GameState TestingConfig::GetAttackTestState()
{
    GameState state;

    return state;
}

GameState TestingConfig::GetSnipeTestState()
{
    GameState state;

    return state;
}

GameState TestingConfig::GetChillTestState()
{
    GameState state;

    return state;
}

GameState TestingConfig::GetFrontlineTestState()
{
    GameState state;   

    return state;
}
