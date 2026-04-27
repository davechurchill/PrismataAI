#pragma once

#include "Common.h"
#include "Eval.h"
#include "Player.h"
#include "Timer.h"

namespace Prismata
{

class PlanEvolutionParameters
{
    PlayerID    _maxPlayer = Players::Player_One;
    size_t      _timeLimit = 1000;
    size_t      _populationSize = 32;
    size_t      _eliteSize = 8;
    size_t      _maxGenerations = 0;
    size_t      _maxActions = MAX_MOVE_ACTIONS;
    int         _evalMethod = EvaluationMethods::WillScoreInflation;
    double      _mutationRate = 0.25;
    double      _mutationStrength = 0.35;

    PlayerPtr   _playoutPlayers[2];

public:

    PlanEvolutionParameters clone() const;

    PlayerID maxPlayer() const { return _maxPlayer; }
    size_t timeLimit() const { return _timeLimit; }
    size_t populationSize() const { return _populationSize; }
    size_t eliteSize() const { return _eliteSize; }
    size_t maxGenerations() const { return _maxGenerations; }
    size_t maxActions() const { return _maxActions; }
    int evalMethod() const { return _evalMethod; }
    double mutationRate() const { return _mutationRate; }
    double mutationStrength() const { return _mutationStrength; }
    const PlayerPtr & getPlayoutPlayer(const PlayerID p) const { return _playoutPlayers[p]; }

    void setMaxPlayer(const PlayerID player) { _maxPlayer = player; }
    void setTimeLimit(const size_t timeLimit) { _timeLimit = timeLimit; }
    void setPopulationSize(const size_t populationSize) { _populationSize = populationSize; }
    void setEliteSize(const size_t eliteSize) { _eliteSize = eliteSize; }
    void setMaxGenerations(const size_t maxGenerations) { _maxGenerations = maxGenerations; }
    void setMaxActions(const size_t maxActions) { _maxActions = maxActions; }
    void setEvalMethod(const int evalMethod) { _evalMethod = evalMethod; }
    void setMutationRate(const double mutationRate) { _mutationRate = mutationRate; }
    void setMutationStrength(const double mutationStrength) { _mutationStrength = mutationStrength; }
    void setPlayoutPlayer(const PlayerID p, const PlayerPtr & player) { _playoutPlayers[p] = player; }
};

class Player_PlanEvolution : public Player
{
public:

    struct Plan
    {
        std::vector<double> buyWeights;
        double actionWeights[ActionTypes::NUM_TYPES];
        double econBias;
        double attackBias;
        double defenseBias;
        double frontlineBias;
        double targetBias;
        double spendBias;
        double endActionBias;
        double endActionGrowth;
        double score;
        Move move;
        bool evaluated;

        Plan();
    };

private:

    PlanEvolutionParameters _params;
    unsigned long long      _rngState;
    size_t                  _lastGenerations;
    size_t                  _lastPlansEvaluated;
    double                  _lastBestScore;
    double                  _lastTimeMS;
    std::string             _lastBestDescription;

    bool searchTimeOut(Timer & timer) const;
    unsigned int randomInt();
    size_t randomIndex(const size_t size);
    double randomDouble();
    double randomSigned();
    double randomRange(const double low, const double high);
    void mutateValue(double & value, const double low, const double high);

    Plan makeBalancedPlan(const GameState & state);
    Plan makeSeedPlan(const GameState & state, const size_t seedIndex);
    Plan makeRandomPlan(const GameState & state);
    std::vector<Plan> makeInitialPopulation(const GameState & state);
    Plan crossover(const Plan & a, const Plan & b);
    void mutate(Plan & plan);

    double evaluateState(const GameState & state);
    double evaluatePlan(const GameState & state, Plan & plan);
    void evaluatePopulation(const GameState & state, Timer & timer, std::vector<Plan> & population);
    void compilePlan(const GameState & state, const Plan & plan, Move & move, std::string * description = nullptr);
    double scoreAction(const GameState & state, const Plan & plan, const Action & action, const size_t phaseActions) const;
    void addEndPhaseIfLegal(GameState & state, std::vector<Action> & actions) const;
    std::string actionSummary(const GameState & state, const Action & action) const;
    std::string getPlanDescription(const GameState & state, const Plan & plan) const;

public:

    Player_PlanEvolution(const PlayerID playerID, const PlanEvolutionParameters & params);

    void getMove(const GameState & state, Move & move);
    PlanEvolutionParameters & getParams();
    std::string getDescription();
    PlayerPtr clone();
};

}
