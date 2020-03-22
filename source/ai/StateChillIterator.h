#pragma once

#include "Common.h"
#include "GameState.h"
#include "Heuristics.h"
#include "ChillScenario.h"
#include "IsomorphicCardSet.h"

namespace Prismata
{

class StateChillIsomorphism
{
    int _blockerCounts[10];
    int _chillerCounts[10];

public:

    StateChillIsomorphism();
    void incBlocker(size_t index);
    void decBlocker(size_t index);
    void incChiller(size_t index);
    void decChiller(size_t index);

    bool blockerCountsEqual(const StateChillIsomorphism & other) const;
    bool chillerCountsEqual(const StateChillIsomorphism & other) const;

    bool operator == (const StateChillIsomorphism & other) const;
    bool dominates(const StateChillIsomorphism & other) const;
};

class StateChillEvaluation
{
    double  _damageEvaluation;
    size_t  _actionsPerformed;

public:

    StateChillEvaluation();
    StateChillEvaluation(double e, size_t a);

    bool betterThan(const StateChillEvaluation & other) const;
};

class StateChillIterator 
{
    const GameState &       _initialState;
    GameState               _currentState;
    const PlayerID          _player;
    const PlayerID          _enemy;
    double                  _initialStateEval;
    GameState               _bestState;
    Move                    _bestMove;
    StateChillEvaluation    _bestEval;
    size_t                  _maxIterations;
    StateChillIsomorphism   _currentStateIsomorphism;
    Move                    _actionStack;
    std::vector<CardID>     _chillers;
    std::vector<CardID>     _blockers;
    bool                    _solved;
    size_t                  _nodesSearched;
    size_t                  _numFrozen;

    std::vector<IsomorphicCardSet>      _isomorphicBlockers;
    std::vector<IsomorphicCardSet>      _isomorphicChillers;
    std::vector<GameState>              _allStates;
    std::vector<StateChillIsomorphism>  _allStateIsomorphisms;

    
    void evaluateState(const GameState & state, const Move & move);
    void reset();
    void printActions(const Move & move);
    void printStackEnglish();
    void printDebug();
    void undoChillTarget(const Card & chiller, const CardID chillerIndex, const Card & blocker, const CardID blockerIndex);
    void recurse(const size_t currentBlockerIndex);
    void recurseIsomorphic(const size_t currentBlockerIndex, const size_t currentChillerIndex, bool partialChill);
    bool checkState(const GameState & state);
    bool checkStateIsomorphism();
    bool chillTarget(const Card & chiller, const CardID chillerIndex, const Card & blocker, const CardID blockerIndex);

public:
    
    StateChillIterator(const GameState & initialState);

    const size_t & getNodesSearched() const;
    bool isSolved() const;

    void solve(size_t maxIterations = 0);
    void solveAll();
    void solveIsomorphic();
    const Move & getBestMove() const;
};
}