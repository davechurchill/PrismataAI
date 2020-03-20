#pragma once

#include "Common.h"

#include "CardType.h"
#include "Move.h"
#include "GameState.h"
#include "Player.h"

namespace Prismata
{

class PuzzleSolver
{
    const std::string       _puzzleFile;
    size_t                  _movesSearched;
    size_t                  _actionsSearched;
    size_t                  _solutionsFound;
    size_t                  _maxDepth;

    bool                    _puzzlePlayerSet;
    bool                    _zeroAttackHeuristic;
    bool                    _attackBlockHeuristic;
    bool                    _turnsToKillHeuristic;
    bool                    _activateAllHeuristic;
    PlayerPtr               _puzzlePlayer;
    GameState               _initialState;

    FILE *                  _resultsFile;

    std::vector<Move>       _lastSolution;

    std::vector<Move>       _moveStack;
    std::vector<GameState>  _stateStack;

    bool                    _printSolutions;

    void recurse(size_t depth);
    void printLastSolution();

    void print(const char * msg, ...);
    bool isTerminalState(const GameState & state, size_t depth);
    
public:

    PuzzleSolver(const std::string & puzzleFile);
    PuzzleSolver(const GameState & state);
    ~PuzzleSolver();

    const GameState & getInitialState() const;
    void setOutputFile(const std::string & filename);
    void setPuzzlePlayer(const std::string & playerName);
    void setMaxDepth(const size_t & maxDepth);
    void setZeroAttackHeuristic(bool val);
    void setAttackBlockHeuristic(bool val);
    void setTurnsToKillHeuristic(bool val);
    void setBetterAttackEstimate(bool val);

    void solve();
};
}