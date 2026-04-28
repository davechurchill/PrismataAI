#include "Tournament.h"
#include "TestingConfig.h"
#include "Timer.h"
#include "PrismataAI.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <chrono>
#include <future>
#include <thread>
using namespace Prismata;

Tournament::Tournament(const rapidjson::Value & tournamentValue)
    : _totalGamesPlayed(0)
    , _discardedGames(0)
    , _updateIntervalSec(0)
    , _randomCards(8)
    , _threads(1)
{
    PRISMATA_ASSERT(tournamentValue.HasMember("name"), "Tournament has no name");
    PRISMATA_ASSERT(tournamentValue.HasMember("rounds"), "Tournament has no rounds number");
    PRISMATA_ASSERT(tournamentValue.HasMember("players"), "Tournament has no players");

    JSONTools::ReadString("name", tournamentValue, _name);
    JSONTools::ReadInt("rounds", tournamentValue, _rounds);
    JSONTools::ReadInt("RandomCards", tournamentValue, _randomCards);
    JSONTools::ReadInt("UpdateIntervalSec", tournamentValue, _updateIntervalSec);
    JSONTools::ReadInt("Threads", tournamentValue, _threads);
    _threads = std::max<size_t>(1, _threads);
    
    PRISMATA_ASSERT(tournamentValue["players"].Size() >= 2, "Tournament has less than 2 players");

    for (size_t i(0); i < tournamentValue["players"].Size(); ++i)
    {
        _players.push_back(tournamentValue["players"][i]["name"].GetString());
        _playerGroups.push_back(tournamentValue["players"][i]["group"].GetInt());
    }
}

void Tournament::run()
{
    auto time = std::time(nullptr);
    auto tm = *std::localtime(&time);

    std::stringstream startDate;
    startDate << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    _date = startDate.str();

    _totalGames = std::vector<int>(_players.size(), 0);
    _totalWins = std::vector<int>(_players.size(), 0);
    _totalDraws = std::vector<int>(_players.size(), 0);
    _totalTurns = std::vector<int>(_players.size(), 0);
    _totalPlayouts = std::vector<int>(_players.size(), 0);
    _totalTimeMS = std::vector<int>(_players.size(), 0);
    _maxTimeMS = std::vector<int>(_players.size(), 0);
    _numGames = std::vector< std::vector<int> >(_players.size(), std::vector<int>(_players.size(), 0));
    _wins = std::vector< std::vector<int> >(_players.size(), std::vector<int>(_players.size(), 0));
    _draws = std::vector< std::vector<int> >(_players.size(), std::vector<int>(_players.size(), 0));
    _turns = std::vector< std::vector<int> >(_players.size(), std::vector<int>(_players.size(), 0));

    size_t totalGamesExpected = 0;
    for (size_t p1(0); p1 < _players.size(); ++p1)
    {
        for (size_t p2(0); p2 < _players.size(); ++p2)
        {
            if (_playerGroups[p1] != _playerGroups[p2])
            {
                totalGamesExpected += 2;
            }
        }
    }
    totalGamesExpected *= _rounds;

    std::cout << "\nStarting tournament " << _name << ": " << _rounds << " rounds, "
              << _players.size() << " players, " << totalGamesExpected
              << " games, " << _threads << " thread" << (_threads == 1 ? "" : "s")
              << ", updates every " << _updateIntervalSec << " seconds" << std::endl;

    Timer t;
    t.start();
    _timeElapsed.start();

    if (_threads == 1)
    {
        for (size_t r(0); r < _rounds; ++r)
        {
            GameState state;
            state.setStartingState(Players::Player_One, _randomCards);

            for (size_t p1(0); p1 < _players.size(); ++p1)
            {
                for (size_t p2(0); p2 < _players.size(); ++p2)
                {
                    if (_playerGroups[p1] == _playerGroups[p2])
                    {
                        continue;
                    }

                    PlayerPtr w1 = AIParameters::Instance().getPlayer(Players::Player_One, _players[p1]);
                    PlayerPtr b1 = AIParameters::Instance().getPlayer(Players::Player_Two, _players[p2]);
                    PlayerPtr w2 = AIParameters::Instance().getPlayer(Players::Player_One, _players[p2]);
                    PlayerPtr b2 = AIParameters::Instance().getPlayer(Players::Player_Two, _players[p1]);

                    TournamentGame g1(state, _players[p1], w1, _players[p2], b1);
                    TournamentGame g2(state, _players[p2], w2, _players[p1], b2);

                    playGame(g1, t);
                    playGame(g2, t);
                }
            }
        }
    }
    else
    {
        std::vector<std::future<TournamentGame>> games;

        auto printUpdate = [&]()
        {
            printResults();
            writeHTMLResults();
            std::cout << std::endl << std::flush;
            t.start();
        };

        auto maybePrintUpdate = [&]()
        {
            if (_updateIntervalSec > 0 && t.getElapsedTimeInSec() >= _updateIntervalSec)
            {
                printUpdate();
            }
        };

        auto finishGame = [&](TournamentGame & game)
        {
            if (game.wasDiscarded())
            {
                discardTournamentGameResult(game);
                maybePrintUpdate();
                return;
            }

            parseTournamentGameResult(game);
            _totalGamesPlayed++;

            if (_updateIntervalSec == 0)
            {
                printUpdate();
            }
            else
            {
                maybePrintUpdate();
            }
        };

        auto collectFinishedGames = [&]()
        {
            bool collected = false;
            for (size_t i(0); i < games.size();)
            {
                if (games[i].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready)
                {
                    TournamentGame game = games[i].get();
                    finishGame(game);
                    games.erase(games.begin() + i);
                    collected = true;
                }
                else
                {
                    ++i;
                }
            }

            return collected;
        };

        auto waitForGameSlot = [&]()
        {
            while (games.size() >= _threads)
            {
                if (!collectFinishedGames())
                {
                    maybePrintUpdate();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
        };

        auto submitGame = [&](const GameState & state, const size_t whiteIndex, const size_t blackIndex)
        {
            waitForGameSlot();
            games.emplace_back(std::async(std::launch::async, [this, state, whiteIndex, blackIndex]()
            {
                return playGame(state, whiteIndex, blackIndex);
            }));
        };

        for (size_t r(0); r < _rounds; ++r)
        {
            GameState state;
            state.setStartingState(Players::Player_One, _randomCards);

            for (size_t p1(0); p1 < _players.size(); ++p1)
            {
                for (size_t p2(0); p2 < _players.size(); ++p2)
                {
                    if (_playerGroups[p1] == _playerGroups[p2])
                    {
                        continue;
                    }

                    submitGame(state, p1, p2);
                    submitGame(state, p2, p1);
                }
            }
        }

        while (!games.empty())
        {
            if (!collectFinishedGames())
            {
                maybePrintUpdate();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    printResults();
    writeHTMLResults();
    std::cout << std::endl << "Tournament complete" << std::endl;
}

TournamentGame Tournament::playGame(const GameState & state, const size_t whiteIndex, const size_t blackIndex) const
{
    PlayerPtr white = AIParameters::Instance().getPlayer(Players::Player_One, _players[whiteIndex]);
    PlayerPtr black = AIParameters::Instance().getPlayer(Players::Player_Two, _players[blackIndex]);

    TournamentGame game(state, _players[whiteIndex], white, _players[blackIndex], black);
    game.playGame();

    return game;
}

void Tournament::playGame(TournamentGame & game, Timer & updateTimer)
{
    game.playGame(_updateIntervalSec);

    if (game.wasDiscarded())
    {
        discardTournamentGameResult(game);

        if (_updateIntervalSec == 0 || updateTimer.getElapsedTimeInSec() >= _updateIntervalSec)
        {
            printResults();
            writeHTMLResults();
            std::cout << std::endl << std::flush;
            updateTimer.start();
        }

        return;
    }

    parseTournamentGameResult(game);

    _totalGamesPlayed++;

    if (_updateIntervalSec == 0 || updateTimer.getElapsedTimeInSec() >= _updateIntervalSec)
    {
        printResults();
        writeHTMLResults();
        std::cout << std::endl << std::flush;
        updateTimer.start();
    }
}

void Tournament::discardTournamentGameResult(const TournamentGame & game)
{
    _discardedGames++;
    std::cout << "Discarded game: " << game.getPlayerName(0) << " vs " << game.getPlayerName(1)
              << " (" << game.getDiscardReason() << ")" << std::endl;
}

void Tournament::parseTournamentGameResult(const TournamentGame & game)
{
    int winnerID = game.getFinalGameState().winner();
    int loserID = (game.getFinalGameState().winner() + 1) % 2;

    int playerIndex[2] = {getPlayerIndex(game.getPlayerName(0)), getPlayerIndex(game.getPlayerName(1))};

    _maxTimeMS[playerIndex[0]] = std::max(_maxTimeMS[playerIndex[0]], (int)game.getMaxTimeMS(0));
    _maxTimeMS[playerIndex[1]] = std::max(_maxTimeMS[playerIndex[1]], (int)game.getMaxTimeMS(1));
    _totalTimeMS[playerIndex[0]] += game.getTotalTimeMS(0);
    _totalTimeMS[playerIndex[1]] += game.getTotalTimeMS(1);
    _totalGames[playerIndex[0]]++;
    _totalGames[playerIndex[1]]++;
    _numGames[playerIndex[0]][playerIndex[1]]++;
    _numGames[playerIndex[1]][playerIndex[0]]++;
    _totalTurns[playerIndex[0]] += game.getFinalGameState().getTurnNumber()/2;
    _totalTurns[playerIndex[1]] += game.getFinalGameState().getTurnNumber()/2;
    _turns[playerIndex[0]][playerIndex[1]] += game.getFinalGameState().getTurnNumber();
    _turns[playerIndex[1]][playerIndex[0]] += game.getFinalGameState().getTurnNumber();


    // case of a draw
    if (winnerID == Players::Player_None)
    {
        _draws[playerIndex[0]][playerIndex[1]]++;
        _draws[playerIndex[1]][playerIndex[0]]++;
        _totalDraws[playerIndex[0]]++;
        _totalDraws[playerIndex[1]]++;
    }
    else
    {
        // case of a non-draw
        int winnerIndex = playerIndex[winnerID];
        int loserIndex = playerIndex[loserID];

        _totalWins[winnerIndex]++;
        _wins[winnerIndex][loserIndex]++;
    }
}

#include "HTMLTable.h"
void Tournament::writeHTMLResults()
{
    std::string filename = "tests/Tournament_" + _name + "_" + _date + ".html";

    std::string assertLevel = "No Asserts";

#ifdef PRISMATA_ASSERT_NORMAL
    assertLevel = "Normal Asserts";
#endif

#ifdef PRISMATA_ASSERT_ALL
    assertLevel = "All Asserts";
#endif
    
    std::stringstream ss;
    double timeElapsed = _timeElapsed.getElapsedTimeInMilliSec();

    ss << "<table cellpadding=2 rules=all style=\"font: 12px/1.5em Verdana; border: 1px solid #cccccc;\">\n";
    ss << "<tr><td width=150><b>Tournament Name</b></td><td width=200 align=right>" << _name << "</td></tr>\n";
    ss << "<tr><td><b>Date Started</b></td><td align=right>" << _date << "</td></tr>\n";
    ss << "<tr><td><b>AI Compiled</b></td><td align=right>" << __DATE__ << " " __TIME__ << "</td></tr>";
    ss << "<tr><td><b>Assert Level</b></td><td align=right>" << assertLevel << "</td></tr>";
    ss << "<tr><td><b>Tournament Rounds</b></td><td align=right>" << _rounds << "</td></tr>\n";
    ss << "<tr><td><b>Time Elapsed</b></td><td align=right>" << getTimeStringFromMS(timeElapsed) << "</td></tr>\n";
    ss << "<tr><td><b>Games Played</b></td><td align=right>" << _totalGamesPlayed << " (" << (1000.0 * _totalGamesPlayed / timeElapsed) << "/s)</td></tr>\n";
    ss << "<tr><td><b>Games Discarded</b></td><td align=right>" << _discardedGames << "</td></tr>\n";
    ss << "</table>\n<br><br>\n";

    FILE * f = fopen(filename.c_str(), "w");
    fprintf(f, "<html>\n<head>\n");
    fprintf(f, "<script type=\"text/javascript\" src=\"javascript/jquery-1.10.2.min.js\"></script>\n<script type=\"text/javascript\" src=\"javascript/jquery.tablesorter.js\"></script>\n<link rel=\"stylesheet\" href=\"javascript/themes/blue/style.css\" type=\"text/css\" media=\"print, projection, screen\" />\n");
    fprintf(f, "</head>\n");
    fprintf(f, ss.str().c_str());
    fclose(f);

    HTMLTable stats("Overall Statistics");
    stats.setHeader({"Player", "Score", "Games", "Wins", "Loss", "Draw", "Turns", "Turns/G", "MS/Turn", "Max MS"});
    stats.setColWidth({120, 80, 80, 80, 80, 80, 80, 80, 80, 80, 80});

    for (size_t p(0); p < _players.size(); ++p)
    {
        size_t col = 0;
        stats.setData(p, col++, _players[p]);
        stats.setData(p, col++, (_totalWins[p] + 0.5*_totalDraws[p])/_totalGames[p]);
        stats.setData(p, col++, _totalGames[p]);
        stats.setData(p, col++, _totalWins[p]);
        stats.setData(p, col++, _totalGames[p] - _totalWins[p] - _totalDraws[p]);
        stats.setData(p, col++, _totalDraws[p]);
        stats.setData(p, col++, _totalTurns[p]);
        stats.setData(p, col++, (double)_totalTurns[p] / _totalGames[p]);
        stats.setData(p, col++, (double)_totalTimeMS[p] / _totalTurns[p]);
        stats.setData(p, col++, _maxTimeMS[p]);
    }

    HTMLTable turnTable("Bot vs. Bot Avg Turns Per Game");
    HTMLTable tableWinPerc("Bot vs. Bot Score Table (row score vs. column)");
    std::vector<std::string> header = {""};
    header.insert(header.end(), _players.begin(), _players.end());
    header.push_back("Total");
    turnTable.setHeader(header);
    tableWinPerc.setHeader(header);
    
    std::vector<size_t> colWidth(header.size(), 120);
    turnTable.setColWidth(colWidth);
    tableWinPerc.setColWidth(colWidth);

    for (size_t r(0); r < _players.size(); ++r)
    {
        size_t col = 0;
        turnTable.setData(r, col, _players[r]);
        tableWinPerc.setData(r, col, _players[r]);
        col++;

        for (size_t p(0); p < _players.size(); ++p)
        {
            if (r == p)
            {
                turnTable.setData(r, col, "-");
                tableWinPerc.setData(r, col, "-");
            }
            else
            {
                turnTable.setData(r, col, _numGames[r][p] == 0 ? 0 : (double)_turns[r][p] / _numGames[r][p]);
                tableWinPerc.setData(r, col, _numGames[r][p] == 0 ? 0 : ((double)_wins[r][p] + 0.5*_draws[r][p]) / _numGames[r][p]);
            }

            col++;
        }

        turnTable.setData(r, col, _totalTurns[r]);
        tableWinPerc.setData(r, col, _totalGames[r] == 0 ? 0 : ((double)_totalWins[r] + 0.5*_totalDraws[r]) / _totalGames[r]);
        col++;
    }

    stats.appendHTMLTableToFile(filename, "statsTable");
    tableWinPerc.appendHTMLTableToFile(filename, "winPercentageTable");
    turnTable.appendHTMLTableToFile(filename, "totalScoreTable");
}

void Tournament::printResults()
{
    std::stringstream ss;
  
    size_t colWidth = 10;
    for (size_t i(0); i < _players.size(); ++i)
    {
        colWidth = std::max(colWidth, _players[i].length() + 2);
    }

    ss << std::endl << std::endl;

    std::stringstream header;
    for (size_t i(0); i < _players.size(); ++i)
    {
        while (header.str().length() < (i+1)*colWidth) header << " ";
        header << _players[i];
    }

    header << "  TotalScore";

    std::cout << header.str() << std::endl;
    ss << header.str() << std::endl;

    for (size_t i(0); i < _players.size(); ++i)
    {
        std::stringstream line;
        line << _players[i]; while (line.str().length() < colWidth) line << " ";

        for (size_t j(0); j < _players.size(); ++j)
        {
            if (_playerGroups[i] != _playerGroups[j])
            {
                line << _wins[i][j] + (0.5*_draws[i][j]) ;
            }
            else
            {
                line << "-";
            }

            while (line.str().length() < colWidth + (j+1)*colWidth) line << " ";
        }

        line << _totalWins[i] + (0.5*_totalDraws[i]);
        line << std::endl;
        ss << line.str();
        std::cout << line.str();
    }

    const double elapsedSec = _timeElapsed.getElapsedTimeInSec();
    const double gamesPerSec = elapsedSec > 0 ? _totalGamesPlayed / elapsedSec : 0.0;

    std::stringstream rate;
    rate << std::fixed << std::setprecision(2) << gamesPerSec;

    std::stringstream threadRate;
    threadRate << std::fixed << std::setprecision(2) << (gamesPerSec / _threads);

    std::cout << "Games completed: " << _totalGamesPlayed << " (" << rate.str() << "/s, " << threadRate.str() << "/s/thread)";
    if (_discardedGames > 0)
    {
        std::cout << ", discarded: " << _discardedGames;
    }
    std::cout << std::endl << std::flush;
}

int Tournament::getPlayerIndex(const std::string & playerName) const
{
    for (size_t i(0); i < _players.size(); ++i)
    {
        if (_players[i].compare(playerName) == 0)
        {
            return i;
        }
    }

    return -1;
}


std::string Tournament::getTimeStringFromMS(const size_t ms)
{
    size_t totalSec = ms / 1000;

    size_t sec = totalSec % 60;
    size_t min = (totalSec / 60) % 60;
    size_t hour = (totalSec / 3600);

    std::stringstream ss;
    if (hour > 0)
    {
        ss << hour << "h ";
    }
    if (min > 0)
    {
        ss << min << "m ";
    }

    ss << sec << "s";
    return ss.str();
}
