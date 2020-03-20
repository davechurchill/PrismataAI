#include "PuzzleSolver.h"
#include <iostream>
#include "MoveIterator_All.h"
#include "AllPlayers.h"
#include "AIParameters.h"

using namespace Prismata;

PuzzleSolver::PuzzleSolver(const std::string & puzzleFile)
    : _puzzleFile(puzzleFile)
    , _movesSearched(0)
    , _actionsSearched(0)
    , _solutionsFound(0)
    , _maxDepth(30)
    , _printSolutions(false)
    , _resultsFile(nullptr)
    , _puzzlePlayerSet(false)
    , _zeroAttackHeuristic(false)
    , _attackBlockHeuristic(false)
    , _turnsToKillHeuristic(false)
    , _activateAllHeuristic(false)
{
    rapidjson::Document document;
    bool parsingFailed = document.Parse(FileUtils::ReadFile(puzzleFile).c_str()).HasParseError();

    PRISMATA_ASSERT(!parsingFailed, "Couldn't parse puzzle file");

    PRISMATA_ASSERT(document.HasMember("whiteInitCards") && document["whiteInitCards"].IsArray(), "Puzzle file has no whiteInitCards array");
    PRISMATA_ASSERT(document.HasMember("blackInitCards") && document["blackInitCards"].IsArray(), "Puzzle file has no blackInitCards array");

    const rapidjson::Value & white = document["whiteInitCards"];
    for (size_t i(0); i < white.Size(); ++i)
    {        
        _initialState.addCard(Players::Player_One, CardTypes::GetCardType(white[i][1].GetString()), white[i][0].GetInt(), CardCreationMethod::Manual, 0, 0);
    }

    const rapidjson::Value & black = document["blackInitCards"];
    for (size_t i(0); i < black.Size(); ++i)
    {        
        _initialState.addCard(Players::Player_Two, CardTypes::GetCardType(black[i][1].GetString()), black[i][0].GetInt(), CardCreationMethod::Manual, 0, 0);
    }

    _puzzlePlayer = AIParameters::Instance().getPlayer(Players::Player_Two, "PuzzlePlayer");
}

PuzzleSolver::PuzzleSolver(const GameState & state)
    : _initialState(state)
    , _movesSearched(0)
    , _actionsSearched(0)
    , _solutionsFound(0)
    , _maxDepth(30)
    , _printSolutions(false)
    , _resultsFile(nullptr)
    , _puzzlePlayerSet(false)
    , _zeroAttackHeuristic(false)
    , _attackBlockHeuristic(false)
    , _turnsToKillHeuristic(false)
    , _activateAllHeuristic(false)
{
    _puzzlePlayer = AIParameters::Instance().getPlayer(Players::Player_Two, "PuzzlePlayer");
}

PuzzleSolver::~PuzzleSolver()
{
    if (_resultsFile != nullptr)
    {
        fclose(_resultsFile);
    }
}

void PuzzleSolver::setMaxDepth(const size_t & maxDepth)
{
    _maxDepth = maxDepth;
}

const GameState & PuzzleSolver::getInitialState() const
{
    return _initialState;
}

void PuzzleSolver::solve()
{
    Timer t;
    t.start();
    _stateStack = std::vector<GameState>(100);
    _moveStack = std::vector<Move>(100);
    _stateStack[0] = _initialState;
    
    recurse(0);

    double ms = t.getElapsedTimeInMilliSec();
    print("Solutions Found: %d\n", _solutionsFound);
    print("Total Nodes Expanded: %d\n", _movesSearched);
    print("Search Time: %dms\n\n", (int)ms);

    if (_solutionsFound > 0)
    {
        print("Shortest Solution Found: \n\n");
        printLastSolution();
    }
}

bool PuzzleSolver::isTerminalState(const GameState & state, size_t depth)
{
    if (state.isGameOver() || depth >= _maxDepth)
    {
        if (state.winner() == Players::Player_One)
        {
            _solutionsFound++;
            _lastSolution = _moveStack;
            print("Solution Found! %d\n", depth);
            printLastSolution();
            _maxDepth = depth-1;
        }

        return true;
    }

    // heursitcs for pruning
    if ((state.getActivePlayer() == Players::Player_One) && (state.numCardsBuyable() == 0))
    {
        HealthType attackSum = 0;
        for (auto & cardID : state.getCardIDs(Players::Player_One))
        {
            attackSum += state.getCardByID(cardID).getType().getAttack();
        }

        if (_activateAllHeuristic)
        {
            PartialPlayer_Defense_Default defense(Players::Player_One);
            PartialPlayer_ActionAbility_ActivateAll activateAll(Players::Player_One);

            Move m;
            GameState stateCopy(state);
            defense.getMove(stateCopy, m);
            activateAll.getMove(stateCopy, m);

            attackSum = stateCopy.getAttack(Players::Player_One);
        }

        if (_zeroAttackHeuristic && (attackSum == 0))
        {
            return true;
        }
             
        HealthType maxDefender = 0;
        HealthType enemyTotalHP = 0;
        for (auto & cardID : state.getCardIDs(Players::Player_Two))
        {
            const Card & card = state.getCardByID(cardID);

            enemyTotalHP += card.currentHealth();

            if (card.getType().isFragile())
            {
                continue;
            }

            if (card.currentHealth() > maxDefender)
            {
                maxDefender = card.currentHealth();
            }
        }

        int turnsToWin = ceil((double)enemyTotalHP / attackSum);

        if (_turnsToKillHeuristic && (depth + turnsToWin > _maxDepth))
        {
            return true;
        }

        if (_attackBlockHeuristic && (attackSum < maxDefender))
        {
            return true;
        }
    }

    return false;
}

void PuzzleSolver::recurse(size_t depth)
{
    const GameState & state = _stateStack[depth];

    if (_movesSearched % 100000 == 0)
    {
        print("Moves: %d\n", _movesSearched);
    }

    // terminal condition
    if (isTerminalState(state, depth))
    {
        return;
    }

    if (state.getActivePlayer() == Players::Player_One || !_puzzlePlayerSet)
    {
        // create a new brute force iterator based on this state
        MoveIteratorPtr iter(new MoveIterator_All(state.getActivePlayer()));
        iter->setState(state);

        // generate each child on the next level of the state stack
        while (iter->generateNextChild(_stateStack[depth+1], _moveStack[depth]))
        {
            _movesSearched++;
            
            // recurse to the next depth
            recurse(depth+1);
        }
    }
    else
    {
        Move m;
        _puzzlePlayer->getMove(state, m);
        
        _stateStack[depth+1] = state;
        _stateStack[depth+1].doMove(m);
        _moveStack[depth] = m;
        _movesSearched++;

        recurse(depth+1);
    }
}

void PuzzleSolver::setPuzzlePlayer(const std::string & playerName)
{
    _puzzlePlayer = AIParameters::Instance().getPlayer(Players::Player_Two, playerName);
    _puzzlePlayerSet = true;
}

void PuzzleSolver::printLastSolution()
{
    for (size_t m(0); m < _maxDepth+1; ++m)
    {
        if (_lastSolution[m].size() == 0)
        {
            break;
        }

        print("P%d:   ", (m % 2));

        for (size_t a(0); a < _lastSolution[m].size(); ++a)
        {
            const Action & action = _lastSolution[m].getAction(a);

            if (action.getType() == ActionTypes::END_PHASE)
            {
                print("* ");
            }
            else
            {
                print("%d ", action.getID());
            }
        }

        print("\n");
    }
}

void PuzzleSolver::print(const char * msg, ...)
{
    char messageBuffer[4096] = "";
    if (msg != NULL)
    {
        va_list args;
        va_start(args, msg);
        vsprintf(messageBuffer, msg, args);
        va_end(args);
    }

    if (_resultsFile != nullptr)
    {
        fprintf(_resultsFile, "%s", messageBuffer);
    }

    printf("%s", messageBuffer);
}

void PuzzleSolver::setOutputFile(const std::string & filename)
{
    _resultsFile = fopen(filename.c_str(), "w");
}

void PuzzleSolver::setZeroAttackHeuristic(bool val)
{
    _zeroAttackHeuristic = val;
}

void PuzzleSolver::setAttackBlockHeuristic(bool val)
{
    _attackBlockHeuristic = val;
}

void PuzzleSolver::setTurnsToKillHeuristic(bool val)
{
    _turnsToKillHeuristic = val;
}

void PuzzleSolver::setBetterAttackEstimate(bool val)
{
    _activateAllHeuristic = val;
}