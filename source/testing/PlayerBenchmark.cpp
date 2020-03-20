#include "PlayerBenchmark.h"
#include "Timer.h"
#include "AIParameters.h"
#include "AITools.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

using namespace Prismata;

PlayerBenchmark::PlayerBenchmark(const rapidjson::Value & value)
    : _gamesPlayed(0)
    , _totalGameTurns(0)
    , _timesBought(CardTypes::GetAllCardTypes().size(), std::vector<size_t>(2, 0))
    , _gamesBought(CardTypes::GetAllCardTypes().size(), std::vector<size_t>(2, 0))
    , _gamesWon(CardTypes::GetAllCardTypes().size(), std::vector<size_t>(2, 0))
    , _gameUnitsBought(CardTypes::GetAllCardTypes().size(), std::vector<size_t>(2, 0))
    , _gamesAvailable(CardTypes::GetAllCardTypes().size(), 0)
    , _willScoreCorrect(0)
    , _willScoreAfterDefenseCorrect(0)
    , _playoutCorrect(0)
    , _timeLimitSec(0)
    , _timeElapsed(0)
    , _doClickString(false)
    , _doUnitStats(false)
    , _doPlayoutEvalStats(false)
    , _doSearchStats(false)
    , _doEvalStats(false)
{
    PRISMATA_ASSERT(value.HasMember("name") && value["name"].IsString(), "PlayerBenchmark must have Name string");
    PRISMATA_ASSERT(value.HasMember("PlayerOne") && value["PlayerOne"].IsString(), "PlayerBenchmark must have PlayerOne string");
    PRISMATA_ASSERT(value.HasMember("PlayerTwo") && value["PlayerTwo"].IsString(), "PlayerBenchmark must have PlayerTwo string");
    PRISMATA_ASSERT(value.HasMember("TimeLimitSec") && value["TimeLimitSec"].IsInt(), "PlayerBenchmark must have TimeLimitMS int");
    PRISMATA_ASSERT(value.HasMember("RandomCards") && value["RandomCards"].IsInt(), "PlayerBenchmark must have RandomCards int");
    PRISMATA_ASSERT(value.HasMember("UpdateIntervalSec") && value["UpdateIntervalSec"].IsInt(), "PlayerBenchmark must have UpdateIntervalMS int");

    JSONTools::ReadString("name", value, _benchmarkName);
    JSONTools::ReadString("PlayerOne", value, _playerNames[Players::Player_One]);
    JSONTools::ReadString("PlayerTwo", value, _playerNames[Players::Player_Two]);
    JSONTools::ReadInt("TimeLimitSec", value, _timeLimitSec);
    JSONTools::ReadInt("RandomCards", value, _randomCards);
    JSONTools::ReadInt("UpdateIntervalSec", value, _writeIntervalSec);
    JSONTools::ReadBool("DoClickString", value, _doClickString);
    JSONTools::ReadBool("DoUnitStats", value, _doUnitStats);
    JSONTools::ReadBool("DoSearchStats", value, _doSearchStats);
    JSONTools::ReadBool("DoPlayoutEvalStats", value, _doPlayoutEvalStats);
    JSONTools::ReadBool("DoEvalStats", value, _doEvalStats);

    _players[Players::Player_One] = AIParameters::Instance().getPlayer(Players::Player_One, _playerNames[Players::Player_One]);
    _players[Players::Player_Two] = AIParameters::Instance().getPlayer(Players::Player_Two, _playerNames[Players::Player_Two]);

    _wins[0] = 0;
    _wins[1] = 0;
    _wins[2] = 0;
}

void PlayerBenchmark::run()
{
    auto time = std::time(nullptr);
    auto tm = *std::localtime(&time);

    std::stringstream startDate;
    startDate << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    _date = startDate.str();
    
    std::cout << "\nStarting " << _timeLimitSec << "sec PlayerBenchmark: [" << _playerNames[0] << " vs. " << _playerNames[1] << "] with " << _randomCards << " random cards for "  << std::endl;
    
    printf("\n  TotalGames  TotalTurns  TimeElapsed    Games/Sec   Turns/Game    Turns/Sec     P1Wins     P2Wins      Draws\n");
    printf("  -------------------------------------------------------------------------------------------------------------\n");

    Timer t;
    t.start();

    // keep playing games until the time limit is reached
    while(_timeElapsed < 1000*_timeLimitSec)
    {
        GameState state;
        state.setStartingState(Players::Player_One, _randomCards);

        // initialize all the data structures required for keeping various statistics
        clearUnitStatistics(state);
        clearEvaluationStatistics();

        // set up the game and play it
        Game game(state, _players[0]->clone(), _players[1]->clone());
        while(!game.gameOver())
        {
            if (_doClickString)
            {
                _beginTurnState = game.getState();       
            }

            updateEvaluationStatistics(game);
            updateUnitStatistics(game);
            updateSearchStatistics(game);

            game.playNextTurn();
            _totalGameTurns++;

            // update the various statistics we are collecting
            doClickStringTest(game);
        }

        // record the end of game statistics
        _gamesPlayed++;
        _wins[game.getState().winner()]++;

        // calculate the final statistics for this game
        calculateEvaluationStatistics(game);
        calculateUnitStatistics(game);

        double ms = t.getElapsedTimeInMilliSec();
        if (ms > 1000*_writeIntervalSec)
        {
            _timeElapsed += ms;

            printResults();
            t.start();
            
            printf("%12d%12d%13.2lf%13.2lf%13.2lf%13.2lf%11d%11d%11d\n", _gamesPlayed, _totalGameTurns, _timeElapsed, ((_gamesPlayed / _timeElapsed) * 1000), ((double)_totalGameTurns / _gamesPlayed), ((_totalGameTurns / _timeElapsed) * 1000), _wins[0], _wins[1], (_gamesPlayed - _wins[0] - _wins[1]));
        }
    }
    printResults();
    //fclose(_learningDataFile);
}

void PlayerBenchmark::doClickStringTest(const Game & game)
{
    if (!_doClickString)
    {
        return;
    }

    const PlayerID playerThatMoved = _beginTurnState.getActivePlayer();

    // get the move that was performed
    const Move & move = game.getPreviousMove();

    // get the click string associated with that move
    const std::string moveClickString = AITools::GetClickString(move, _beginTurnState);

    // convert that click string back to a move to test
    const Move & convertedMove = AITools::GetMoveFromClickString(moveClickString, playerThatMoved, _beginTurnState);

    // do the converted move
    _beginTurnState.doMove(convertedMove);

    // assert that the resulting state from doing the converted move is the same as the original move
    PRISMATA_ASSERT(_beginTurnState.isIsomorphic(game.getState()), "States are not isomorphic");
    
}

void PlayerBenchmark::updateUnitStatistics(const Game & game)
{
    if (!_doUnitStats)
    {
        return;
    }

    const Move & previousMove = game.getPreviousMove();
    for (size_t a(0); a < previousMove.size(); ++a)
    {
        const Action & action = previousMove.getAction(a);

        if (action.getType() == ActionTypes::BUY)
        {
            _gameUnitsBought[action.getID()][action.getPlayer()]++;
        }
    }
}

void PlayerBenchmark::clearUnitStatistics(const GameState & state)
{
    if (!_doUnitStats)
    {
        return;
    }

    for (size_t c(0); c < state.numCardsBuyable(); ++c)
    {
        const auto & cb = state.getCardBuyableByIndex(c);

        _gamesAvailable[cb.getType().getID()]++;
    }

    _gameUnitsBought = vvs(CardTypes::GetAllCardTypes().size(), std::vector<size_t>(2, 0));
}

void PlayerBenchmark::calculateUnitStatistics(const Game & game)
{
    if (!_doUnitStats)
    {
        return;
    }

    for (size_t i(0); i < _gameUnitsBought.size(); ++i)
    {
        for (size_t p(0); p < 2; ++p)
        {
            // add to the total times this card was bought
            _timesBought[i][p] += _gameUnitsBought[i][p];

            // add to the toal games this card was bought
            if (_gameUnitsBought[i][p] > 0)
            {
                _gamesBought[i][p]++;

                // add to the total games this card was bought this player won
                if (game.getState().winner() == p)
                {
                    _gamesWon[i][p]++;
                }
            }
        }
    }
}

void PlayerBenchmark::updateSearchStatistics(const Game & game)
{
    if (!_doSearchStats)
    {
        return;
    }

    PlayerID previousPlayer = game.getState().getInactivePlayer();

    Player * player = game.getPlayer(previousPlayer).get();
    Player_StackAlphaBeta * abPlayer = dynamic_cast<Player_StackAlphaBeta *>(player);
    Player_UCT * uctPlayer = dynamic_cast<Player_UCT *>(player);

    if (_evaluationsPerformed.size() < game.getState().getTurnNumber()+1u)
    {
        _nodesExpanded.resize(game.getState().getTurnNumber()+1, 0);
        _evaluationsPerformed.resize(game.getState().getTurnNumber()+1, 0);
        _rootChildrenGenerated.resize(game.getState().getTurnNumber()+1, 0);
        _turnTimeElapsed.resize(game.getState().getTurnNumber()+1, 0);
    }

    // if this is an alpha beta player
    if (abPlayer != nullptr)
    {
        AlphaBetaSearchResults & results = abPlayer->getResults();

        _evaluationsPerformed[game.getState().getTurnNumber()] += results.evaluationsPerformed;
        _nodesExpanded[game.getState().getTurnNumber()] += results.nodesExpanded;
        _rootChildrenGenerated[game.getState().getTurnNumber()] += results.rootNumChildren;
        _turnTimeElapsed[game.getState().getTurnNumber()] += results.totalTimeElapsed;
    }
    // if it's a uct player
    else if (uctPlayer != nullptr)
    {
        UCTSearchResults & results = uctPlayer->getResults();

        _evaluationsPerformed[game.getState().getTurnNumber()] += results.traversals;
        _nodesExpanded[game.getState().getTurnNumber()] += results.nodesCreated;
        _turnTimeElapsed[game.getState().getTurnNumber()] += results.timeElapsed;
    }
    else
    {
    
    }
}

void PlayerBenchmark::clearEvaluationStatistics()
{
    _willScore.clear();
    _willScoreAfterDefense.clear();
    _playoutWinner.clear();
    _productionDiff.clear();
}

void PlayerBenchmark::updateEvaluationStatistics(const Game & game)
{
    if (!_doEvalStats)
    {
        return;
    }

    const GameState & state = game.getState();

    // compute basic will score evaluation
    double evalOne = Eval::WillScoreSum(game.getState(), 0);
    double evalTwo = Eval::WillScoreSum(game.getState(), 1);

    _willScoreAfterDefense.push_back(evalOne - evalTwo);

    if (game.getState().getActivePlayer() == 0)
    {
        evalOne *= 1.13;
    }
    else
    {
        evalTwo *= 1.13;
    }
    
    _willScore.push_back(evalOne - evalTwo);
    //_willScore.push_back(Eval::WillScoreEvaluation(game.getState(), 0));
    
    Resources produce[2];
    for (PlayerID p(0); p < 2; ++p)
    {
        for (auto cardID : state.getCardIDs(p))
        {
            const Card & card = state.getCardByID(cardID);
            
            produce[p].add(card.getType().getBeginOwnTurnScript().getEffect().getReceive());
            produce[p].add(card.getType().getAbilityScript().getEffect().getReceive());
        }
    }

    double m1 = game.getState().getActivePlayer() == 0 ? 1.13 : 1;
    double m2 = game.getState().getActivePlayer() == 0 ? 1 : 1.13;

    ManaDiff productionDiff;
    productionDiff.gold = (int)produce[0].amountOf(Resources::Gold) * m1 - produce[1].amountOf(Resources::Gold) * m2;
    productionDiff.r = (int)produce[0].amountOf(Resources::Red) * m1 - produce[1].amountOf(Resources::Red) * m2;
    productionDiff.g = (int)produce[0].amountOf(Resources::Green) * m1 - produce[1].amountOf(Resources::Green) * m2;
    productionDiff.b = (int)produce[0].amountOf(Resources::Blue) * m1 - produce[1].amountOf(Resources::Blue) * m2;
    productionDiff.a = (int)produce[0].amountOf(Resources::Attack) * m1 - produce[1].amountOf(Resources::Attack) * m2;
    productionDiff.e = (int)produce[0].amountOf(Resources::Energy) * m1 - produce[1].amountOf(Resources::Energy) * m2;

    _productionDiff.push_back(productionDiff);

    // compute will score evaluation after defense phase 
    GameState afterDefenseState(game.getState());
    PartialPlayer_Defense_Solver defenseSolver(afterDefenseState.getActivePlayer());
    Move m;
    defenseSolver.getMove(afterDefenseState, m);
    
    //_willScoreAfterDefense.push_back(Eval::WillScoreEvaluation(afterDefenseState, 0));
    
    // compute playout evaluation
    if (_doPlayoutEvalStats)
    {
        Game playoutGame(game.getState(), AIParameters::Instance().getPlayer(0, "Playout"), AIParameters::Instance().getPlayer(1, "Playout"));
        playoutGame.play();
        _playoutWinner.push_back(playoutGame.getState().winner());
    }
}

void PlayerBenchmark::calculateEvaluationStatistics(const Game & game)
{
    if (!_doEvalStats)
    {
        return;
    }

    PlayerID winner = game.getState().winner();

    // resize the vector if necessary
    if (_willScore.size() > _willScoreCorrectOnTurn.size())
    {
        _willScoreCorrectOnTurn.resize(_willScore.size(), 0);
        _willScoreAfterDefenseCorrectOnTurn.resize(_willScore.size(), 0);
        _playoutCorrectOnTurn.resize(_willScore.size(), 0);
        _totalTurnsReached.resize(_willScore.size(), 0);
        _productionDiff.resize(_willScore.size(), ManaDiff());
        _productionDiffCorrectOnTurn.resize(_willScore.size(), ManaDiff());
    }

    for (size_t i(0); i < _willScore.size(); ++i)
    {
        /*if (winner < 2)
        {
            std::stringstream ss;
            ss << _willScore[i] << ", " << _productionDiff[i].gold << ", " << _productionDiff[i].a << ", " << ((int)winner) << "\n";
            fprintf(_learningDataFile, ss.str().c_str());
        }*/

        _productionDiffCorrectOnTurn[i].gold += (_productionDiff[i].gold == 0) ? 0.5 : 0;
        _productionDiffCorrectOnTurn[i].r += (_productionDiff[i].r == 0) ? 0.5 : 0;
        _productionDiffCorrectOnTurn[i].g += (_productionDiff[i].g == 0) ? 0.5 : 0;
        _productionDiffCorrectOnTurn[i].b += (_productionDiff[i].b == 0) ? 0.5 : 0;
        _productionDiffCorrectOnTurn[i].e += (_productionDiff[i].e == 0) ? 0.5 : 0;
        _productionDiffCorrectOnTurn[i].a += (_productionDiff[i].a == 0) ? 0.5 : 0;
        
        _productionDiffCorrectOnTurn[i].gold += ((winner == Players::Player_One && _productionDiff[i].gold > 0) || (winner == Players::Player_Two && _productionDiff[i].gold < 0)) ? 1 : 0;
        _productionDiffCorrectOnTurn[i].r += ((winner == Players::Player_One && _productionDiff[i].r > 0) || (winner == Players::Player_Two && _productionDiff[i].r < 0)) ? 1 : 0;
        _productionDiffCorrectOnTurn[i].g += ((winner == Players::Player_One && _productionDiff[i].g > 0) || (winner == Players::Player_Two && _productionDiff[i].g < 0)) ? 1 : 0;
        _productionDiffCorrectOnTurn[i].b += ((winner == Players::Player_One && _productionDiff[i].b > 0) || (winner == Players::Player_Two && _productionDiff[i].b < 0)) ? 1 : 0;
        _productionDiffCorrectOnTurn[i].e += ((winner == Players::Player_One && _productionDiff[i].e > 0) || (winner == Players::Player_Two && _productionDiff[i].e < 0)) ? 1 : 0;
        _productionDiffCorrectOnTurn[i].a += ((winner == Players::Player_One && _productionDiff[i].a > 0) || (winner == Players::Player_Two && _productionDiff[i].a < 0)) ? 1 : 0;

        if ((winner == Players::Player_One && _willScore[i] > 0) || (winner == Players::Player_Two && _willScore[i] < 0))
        {
            _willScoreCorrect++;
            _willScoreCorrectOnTurn[i]++;
        }

        if ((winner == Players::Player_One && _willScoreAfterDefense[i] > 0) || (winner == Players::Player_Two && _willScoreAfterDefense[i] < 0))
        {
            _willScoreAfterDefenseCorrect++;
            _willScoreAfterDefenseCorrectOnTurn[i]++;
        }

        _totalTurnsReached[i]++;

        if (_doPlayoutEvalStats)
        {
            if (winner == _playoutWinner[i])
            {
                _playoutCorrectOnTurn[i]++;
                _playoutCorrect++;
            }
        }
    }
}

#include <iostream>
#include <iomanip>
#include <ctime>
#include "HTMLTable.h"
#include "HTMLChart.h"
void PlayerBenchmark::printResults()
{
    std::string filename = "tests/Benchmark_" + _benchmarkName + "_" + _date + ".html";
    const auto & allCards = CardTypes::GetAllCardTypes();
    
    std::string assertLevel = "No Asserts";

#ifdef PRISMATA_ASSERT_NORMAL
    assertLevel = "Normal Asserts";
#endif

#ifdef PRISMATA_ASSERT_ALL
    assertLevel = "All Asserts";
#endif
    
    std::stringstream ss;

    ss << "<table cellpadding=2 rules=all style=\"font: 12px/1.5em Verdana; border: 1px solid #cccccc;\">\n";
    ss << "<tr><td width=150><b>Benchmark Name</b></td><td width=200 align=right>" << _benchmarkName << "</td></tr>\n";
    ss << "<tr><td><b>Date Started</b></td><td align=right>" << _date << "</td></tr>\n";
    ss << "<tr><td><b>AI Compiled</b></td><td align=right>" << __DATE__ << " " __TIME__ << "</td></tr>";
    ss << "<tr><td><b>Assert Level</b></td><td align=right>" << assertLevel << "</td></tr>";
    ss << "<tr><td><b>Clickstring Tested</b></td><td align=right>" << (_doClickString ? "true" : "false") << "</td></tr>\n";
    ss << "<tr><td><b>Unit Statistics</b></td><td align=right>" << (_doUnitStats ? "true" : "false") << "</td></tr>\n";
    ss << "<tr><td><b>Player One</b></td><td align=right>" << _playerNames[0] << "</td></tr>\n";
    ss << "<tr><td><b>Player Two</b></td><td align=right>" << _playerNames[1] << "</td></tr>\n";
    ss << "<tr><td><b>Test Time Limit</b></td><td align=right>" << getTimeStringFromMS(_timeLimitSec*1000) << "</td></tr>\n";
    ss << "<tr><td><b>Test Time Elapsed</b></td><td align=right>" << getTimeStringFromMS(_timeElapsed) << "</td></tr>\n";
    ss << "<tr><td><b>Games Played</b></td><td align=right>" << _gamesPlayed << " (" << (1000.0 * _gamesPlayed / _timeElapsed) << "/s)</td></tr>\n";
    ss << "<tr><td><b>Total Turns</b></td><td align=right>" << _totalGameTurns << " (" << (1000.0 * _totalGameTurns / _timeElapsed) << "/s)</td></tr>\n";
    ss << "<tr><td><b>Will Score Correct</b></td><td align=right>" << ((double)_willScoreCorrect / _totalGameTurns) << "</td></tr>\n";
    ss << "<tr><td><b>Score After Def Correct</b></td><td align=right>" << ((double)_willScoreAfterDefenseCorrect / _totalGameTurns) << "</td></tr>\n";
    ss << "<tr><td><b>Playout Correct</b></td><td align=right>" << ((double)_playoutCorrect / _totalGameTurns) << "</td></tr>\n";
    ss << "<tr><td><b>Player One Wins</b></td><td align=right>" << _wins[0] << " - " << (100.0*_wins[0] / _gamesPlayed) << "%%</td></tr>\n";
    ss << "<tr><td><b>Player Two Wins</b></td><td align=right>" << _wins[1] << " - " << (100.0*_wins[1] / _gamesPlayed) << "%%</td></tr>\n";
    ss << "<tr><td><b>Draws</td><td align=right>" << (_gamesPlayed - _wins[0] - _wins[1]) << "</td></tr>\n";
    ss << "</table>\n<br><br>\n";

    FILE * f = fopen(filename.c_str(), "w");
    fprintf(f, "<html>\n<head>\n");
    fprintf(f, "<script type=\"text/javascript\" src=\"javascript/jquery-1.10.2.min.js\"></script>\n<script type=\"text/javascript\" src=\"javascript/jquery.tablesorter.js\"></script>\n<link rel=\"stylesheet\" href=\"javascript/themes/blue/style.css\" type=\"text/css\" media=\"print, projection, screen\" />\n");
    fprintf(f, "</head>\n");
    fprintf(f, ss.str().c_str());
    fclose(f);
    
    if (_doEvalStats)
    {
        HTMLChart chart("Evaluation Accuracy", "Turn Number", "Percent Correct at Turn T");

        std::vector<double> wsPerc(_willScoreCorrectOnTurn.size(), 0);
        std::vector<double> wsadPerc(_willScoreCorrectOnTurn.size(), 0);
        std::vector<double> playoutPerc(_willScoreCorrectOnTurn.size(), 0);
        std::vector<double> goldPerc(_willScoreCorrectOnTurn.size(), 0);
        std::vector<double> attackPerc(_willScoreCorrectOnTurn.size(), 0);
        for (size_t i(0); i < _willScoreCorrectOnTurn.size(); ++i)
        {
            wsPerc[i] = ((double)_willScoreCorrectOnTurn[i] / _totalTurnsReached[i]);
            wsadPerc[i] = ((double)_willScoreAfterDefenseCorrectOnTurn[i] / _totalTurnsReached[i]);
            playoutPerc[i] = ((double)_playoutCorrectOnTurn[i] / _totalTurnsReached[i]);
            goldPerc[i] = ((double)_productionDiffCorrectOnTurn[i].gold / _totalTurnsReached[i]);
            attackPerc[i] = ((double)_productionDiffCorrectOnTurn[i].a / _totalTurnsReached[i]);
        }

        chart.setData("Will Score Inf", wsPerc);
        chart.setData("Will Score NoInf", wsadPerc);
        chart.setData("Gold Income Diff", goldPerc);
        chart.setData("Attack Diff", attackPerc);

        if (_doPlayoutEvalStats)
        {
            chart.setData("Playout", playoutPerc);
        }

        chart.appendHTMLChartToFile(filename, "EvalChart", 0, 50);
    }

    if (_doSearchStats)
    {
        HTMLChart playoutChart("Average Playouts Performed by Turn Number", "Turn Number", "Playouts Performed");
        std::vector<double> avgEvals(_evaluationsPerformed.size(), 0);
        std::vector<double> avgNodes(_nodesExpanded.size(), 0);
        for (size_t i(0); i < _evaluationsPerformed.size(); ++i)
        {
            avgEvals[i] = (double)_evaluationsPerformed[i] / _totalTurnsReached[i];
            avgNodes[i] = (double)_nodesExpanded[i] / _totalTurnsReached[i];
        }
        playoutChart.setData("Evaluations Performed", avgEvals);
        playoutChart.setData("Nodes Expanded", avgNodes);
            
        playoutChart.appendHTMLChartToFile(filename, "PlayoutChart", 0, 50);

        HTMLChart childChart("Average Children Generated At Root", "Turn Number", "Avg Children");
        std::vector<double> avgChildren(_rootChildrenGenerated.size(), 0);
        for (size_t i(0); i < _rootChildrenGenerated.size(); ++i)
        {
            avgChildren[i] = (double)_rootChildrenGenerated[i] / _totalTurnsReached[i];
        }
        childChart.setData("Num Children", avgChildren);
        childChart.appendHTMLChartToFile(filename, "ChildChart", 0, 50);

        HTMLChart timeChart("Average Think Time Elapsed Per Turn", "Turn Number", "Avg Time");
        std::vector<double> avgTime(_turnTimeElapsed.size(), 0);
        for (size_t i(0); i < _turnTimeElapsed.size(); ++i)
        {
            avgChildren[i] = (double)_turnTimeElapsed[i] / _totalTurnsReached[i];
        }
        timeChart.setData("Avg Time Elapsed (ms)", avgChildren);
        timeChart.appendHTMLChartToFile(filename, "TimeChart", 0, 50);
    }
    
    if (_doUnitStats)
    {
        HTMLTable table("Unit Usage Statistics");
        table.setHeader(    {"Unit Name", "id", "$", "R", "G", "B", "Games", "BuyP1", "BuyP2", "AvgBuyP1", "AvgBuyP2", "GBuyP1", "GBuyP2", "PBuyP1", "PBuyP2", "WinP1", "WinP2", "PWonP1", "PWonP2", "PWinDiff"});
        table.setColWidth(  {150,         30,   30,  30,  30,  30,  90,     90,     90,     90,         90,       90,      90,      90,      90,      90,     90,     90,      90,      90});
    
        for (size_t c(2); c < allCards.size(); ++c)
        {
            const Resources & buyCost = allCards[c].getBuyCost();

            size_t col = 0;
            table.setData(c, col++, allCards[c].getUIName().c_str());
            table.setData(c, col++, c);
            table.setData(c, col++, (size_t)buyCost.amountOf(Resources::Gold));
            table.setData(c, col++, (size_t)buyCost.amountOf(Resources::Red));
            table.setData(c, col++, (size_t)buyCost.amountOf(Resources::Green));
            table.setData(c, col++, (size_t)buyCost.amountOf(Resources::Blue));
            table.setData(c, col++, _gamesAvailable[c]);
            table.setData(c, col++, _timesBought[c][0]);
            table.setData(c, col++, _timesBought[c][1]);
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_timesBought[c][0]/_gamesAvailable[c]));
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_timesBought[c][1]/_gamesAvailable[c]));
            table.setData(c, col++, _gamesBought[c][0]);
            table.setData(c, col++, _gamesBought[c][1]);
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_gamesBought[c][0]/_gamesAvailable[c]));
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_gamesBought[c][1]/_gamesAvailable[c]));
            table.setData(c, col++, _gamesWon[c][0]);
            table.setData(c, col++, _gamesWon[c][1]);
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_gamesWon[c][0]/_gamesBought[c][0]));
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : ((double)_gamesWon[c][1]/_gamesBought[c][1]));
            table.setData(c, col++, _gamesAvailable[c] == 0 ? 0 : fabs(((double)_gamesWon[c][1]/_gamesBought[c][1]) - ((double)_gamesWon[c][0]/_gamesBought[c][0])));
        }
            
        table.appendHTMLTableToFile(filename, "unitStatsTable");
    }
}

std::string PlayerBenchmark::getTimeStringFromMS(const size_t ms)
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
