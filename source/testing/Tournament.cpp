#include "Tournament.h"
#include "TestingConfig.h"
#include "Timer.h"
#include "PrismataAI.h"

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <condition_variable>
#include <deque>
#include <thread>

using namespace Prismata;

Tournament::Tournament(const rapidjson::Value & tournamentValue)
    : _totalGamesPlayed(0)
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

    int threads = 1;
    JSONTools::ReadInt("ParallelGames", tournamentValue, threads);
    JSONTools::ReadInt("Threads", tournamentValue, threads);
    PRISMATA_ASSERT(threads >= 1, "Tournament Threads / ParallelGames must be at least 1");
    _threads = (size_t)threads;
    
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

    _timeElapsed.start();
    _updateTimer.start();

    if (_threads == 1)
    {
        runSerial();
    }
    else
    {
        runParallel();
    }

    {
        std::lock_guard<std::mutex> lock(_resultsMutex);
        printResults();
        writeHTMLResults();
    }
}

void Tournament::runSerial()
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

                GameJob g1;
                g1.initialState = state;
                g1.playerOneIndex = p1;
                g1.playerTwoIndex = p2;

                GameJob g2;
                g2.initialState = state;
                g2.playerOneIndex = p2;
                g2.playerTwoIndex = p1;

                playGameJob(g1);
                playGameJob(g2);
            }
        }
    }
}

void Tournament::runParallel()
{
    std::deque<GameJob> jobs;
    std::mutex jobsMutex;
    std::condition_variable jobsChanged;
    std::condition_variable queueHasSpace;
    bool allJobsQueued = false;

    const size_t maxQueuedGames = std::max<size_t>(_threads * 4, _threads);

    auto queueGame = [&](const GameJob & job)
    {
        std::unique_lock<std::mutex> lock(jobsMutex);
        queueHasSpace.wait(lock, [&]() { return jobs.size() < maxQueuedGames; });
        jobs.push_back(job);
        jobsChanged.notify_one();
    };

    auto worker = [&]()
    {
        while (true)
        {
            GameJob job;

            {
                std::unique_lock<std::mutex> lock(jobsMutex);
                jobsChanged.wait(lock, [&]() { return allJobsQueued || !jobs.empty(); });

                if (jobs.empty())
                {
                    return;
                }

                job = jobs.front();
                jobs.pop_front();
                queueHasSpace.notify_one();
            }

            playGameJob(job);
        }
    };

    std::vector<std::thread> workers;
    for (size_t t(0); t < _threads; ++t)
    {
        workers.push_back(std::thread(worker));
    }

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

                GameJob g1;
                g1.initialState = state;
                g1.playerOneIndex = p1;
                g1.playerTwoIndex = p2;

                GameJob g2;
                g2.initialState = state;
                g2.playerOneIndex = p2;
                g2.playerTwoIndex = p1;

                queueGame(g1);
                queueGame(g2);
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(jobsMutex);
        allJobsQueued = true;
    }

    jobsChanged.notify_all();

    for (size_t t(0); t < workers.size(); ++t)
    {
        workers[t].join();
    }
}

void Tournament::playGame(TournamentGame & game)
{
    game.playGame();

    std::lock_guard<std::mutex> lock(_resultsMutex);

    parseTournamentGameResult(game);

    _totalGamesPlayed++;

    if (_updateTimer.getElapsedTimeInSec() > _updateIntervalSec)
    {
        printResults();
        writeHTMLResults();
        printf("\n\n");
        _updateTimer.start();
    }
}

void Tournament::playGameJob(const GameJob & job)
{
    PlayerPtr playerOne;
    PlayerPtr playerTwo;

    {
        std::lock_guard<std::mutex> lock(_playerCreationMutex);
        playerOne = AIParameters::Instance().getPlayer(Players::Player_One, _players[job.playerOneIndex]);
        playerTwo = AIParameters::Instance().getPlayer(Players::Player_Two, _players[job.playerTwoIndex]);
    }

    TournamentGame game(job.initialState, _players[job.playerOneIndex], playerOne, _players[job.playerTwoIndex], playerTwo);
    playGame(game);
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
    double gamesPerSec = timeElapsed > 0 ? (1000.0 * _totalGamesPlayed / timeElapsed) : 0;

    ss << "<table cellpadding=2 rules=all style=\"font: 12px/1.5em Verdana; border: 1px solid #cccccc;\">\n";
    ss << "<tr><td width=150><b>Tournament Name</b></td><td width=200 align=right>" << _name << "</td></tr>\n";
    ss << "<tr><td><b>Date Started</b></td><td align=right>" << _date << "</td></tr>\n";
    ss << "<tr><td><b>AI Compiled</b></td><td align=right>" << __DATE__ << " " __TIME__ << "</td></tr>";
    ss << "<tr><td><b>Assert Level</b></td><td align=right>" << assertLevel << "</td></tr>";
    ss << "<tr><td><b>Tournament Rounds</b></td><td align=right>" << _rounds << "</td></tr>\n";
    ss << "<tr><td><b>Threads</b></td><td align=right>" << _threads << "</td></tr>\n";
    ss << "<tr><td><b>Time Elapsed</b></td><td align=right>" << getTimeStringFromMS(timeElapsed) << "</td></tr>\n";
    ss << "<tr><td><b>Games Played</b></td><td align=right>" << _totalGamesPlayed << " (" << gamesPerSec << "/s)</td></tr>\n";
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
        double score = _totalGames[p] == 0 ? 0 : (_totalWins[p] + 0.5*_totalDraws[p])/_totalGames[p];
        double turnsPerGame = _totalGames[p] == 0 ? 0 : (double)_totalTurns[p] / _totalGames[p];
        double msPerTurn = _totalTurns[p] == 0 ? 0 : (double)_totalTimeMS[p] / _totalTurns[p];

        stats.setData(p, col++, _players[p]);
        stats.setData(p, col++, score);
        stats.setData(p, col++, _totalGames[p]);
        stats.setData(p, col++, _totalWins[p]);
        stats.setData(p, col++, _totalGames[p] - _totalWins[p] - _totalDraws[p]);
        stats.setData(p, col++, _totalDraws[p]);
        stats.setData(p, col++, _totalTurns[p]);
        stats.setData(p, col++, turnsPerGame);
        stats.setData(p, col++, msPerTurn);
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

void Tournament::printResults() const
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
