#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"

namespace Prismata
{

class Timer;

class RootRacingParameters
{
    PlayerID                    _maxPlayer = Players::Player_One;
    size_t                      _timeLimit = 0;
    size_t                      _maxCandidates = 80;
    size_t                      _maxRacePasses = 1;
    double                      _keepFraction = 0.5;
    bool                        _roundRobinRootIterators = false;
    int                         _evalMethod = EvaluationMethods::Playout;
    int                         _seedEvalMethod = EvaluationMethods::WillScore;

    std::vector<MoveIteratorPtr> _rootMoveIterators;
    PlayerPtr                    _playoutPlayers[2];

public:

    PlayerID maxPlayer() const { return _maxPlayer; }
    size_t timeLimit() const { return _timeLimit; }
    size_t maxCandidates() const { return _maxCandidates; }
    size_t maxRacePasses() const { return _maxRacePasses; }
    double keepFraction() const { return _keepFraction; }
    bool roundRobinRootIterators() const { return _roundRobinRootIterators; }
    int evalMethod() const { return _evalMethod; }
    int seedEvalMethod() const { return _seedEvalMethod; }

    const std::vector<MoveIteratorPtr> & getRootMoveIterators() const { return _rootMoveIterators; }
    const PlayerPtr & getPlayoutPlayer(const PlayerID p) const { return _playoutPlayers[p]; }

    void setMaxPlayer(const PlayerID player) { _maxPlayer = player; }
    void setTimeLimit(const size_t timeLimit) { _timeLimit = timeLimit; }
    void setMaxCandidates(const size_t maxCandidates) { _maxCandidates = maxCandidates; }
    void setMaxRacePasses(const size_t maxRacePasses) { _maxRacePasses = maxRacePasses; }
    void setKeepFraction(const double keepFraction) { _keepFraction = keepFraction; }
    void setRoundRobinRootIterators(const bool roundRobinRootIterators) { _roundRobinRootIterators = roundRobinRootIterators; }
    void setEvalMethod(const int evalMethod) { _evalMethod = evalMethod; }
    void setSeedEvalMethod(const int seedEvalMethod) { _seedEvalMethod = seedEvalMethod; }
    void addRootMoveIterator(const MoveIteratorPtr & iterator) { _rootMoveIterators.push_back(iterator); }
    void setPlayoutPlayer(const PlayerID p, const PlayerPtr & ptr) { _playoutPlayers[p] = ptr; }

    RootRacingParameters clone() const;
    std::string getDescription() const;
};

class Player_RootRacing : public Player
{
public:

    struct Candidate
    {
        Move        move;
        GameState   state;
        double      seedScore = 0;
        double      scoreSum = 0;
        size_t      visits = 0;
        std::string description;

        double score() const { return visits > 0 ? scoreSum / visits : seedScore; }
    };

private:

    RootRacingParameters     _params;
    size_t                   _lastCandidatesGenerated = 0;
    size_t                   _lastCandidatesAfterDedupe = 0;
    size_t                   _lastRacePasses = 0;
    size_t                   _lastEvaluations = 0;
    double                   _lastTimeMS = 0;
    double                   _lastBestScore = 0;
    std::string              _lastBestDescription;

    bool        searchTimeOut(Timer & timer) const;
    bool        candidateExists(const std::vector<Candidate> & candidates, const Move & move, const GameState & state) const;
    double      evaluateState(const GameState & state, const int evalMethod);
    void        generateCandidates(const GameState & state, Timer & timer, std::vector<Candidate> & candidates);
    void        generateCandidatesSequential(const GameState & state, Timer & timer, std::vector<Candidate> & candidates);
    void        generateCandidatesRoundRobin(const GameState & state, Timer & timer, std::vector<Candidate> & candidates);
    void        seedCandidates(std::vector<Candidate> & candidates);
    void        raceCandidates(Timer & timer, std::vector<Candidate> & candidates);
    size_t      keepCount(const size_t candidates) const;

public:

    Player_RootRacing(const PlayerID playerID, const RootRacingParameters & params);

    void getMove(const GameState & state, Move & move);
    RootRacingParameters & getParams();
    std::string getDescription();
    PlayerPtr clone();
};

}
