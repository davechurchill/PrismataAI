#include "Player_PlanEvolution.h"

#include "CardTypes.h"
#include "Heuristics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

using namespace Prismata;

namespace
{
    const double MinActionScore = -1000000000.0;

    bool PlanScoreCompare(const Player_PlanEvolution::Plan & lhs, const Player_PlanEvolution::Plan & rhs)
    {
        if (lhs.evaluated != rhs.evaluated)
        {
            return lhs.evaluated;
        }

        return lhs.score > rhs.score;
    }

    double ClampDouble(const double value, const double low, const double high)
    {
        return std::max(low, std::min(high, value));
    }
}

PlanEvolutionParameters PlanEvolutionParameters::clone() const
{
    PlanEvolutionParameters params(*this);

    for (PlayerID p(0); p < 2; ++p)
    {
        params._playoutPlayers[p] = _playoutPlayers[p].get() ? _playoutPlayers[p]->clone() : PlayerPtr();
    }

    return params;
}

Player_PlanEvolution::Plan::Plan()
    : econBias(1.0)
    , attackBias(1.0)
    , defenseBias(1.0)
    , frontlineBias(1.0)
    , targetBias(1.0)
    , spendBias(1.0)
    , endActionBias(-2.0)
    , endActionGrowth(0.4)
    , score(-std::numeric_limits<double>::max())
    , evaluated(false)
{
    for (size_t i(0); i < ActionTypes::NUM_TYPES; ++i)
    {
        actionWeights[i] = 0.0;
    }

    actionWeights[ActionTypes::USE_ABILITY] = 5.0;
    actionWeights[ActionTypes::BUY] = 4.0;
    actionWeights[ActionTypes::ASSIGN_BLOCKER] = 7.0;
    actionWeights[ActionTypes::ASSIGN_BREACH] = 6.0;
    actionWeights[ActionTypes::ASSIGN_FRONTLINE] = 4.0;
    actionWeights[ActionTypes::SNIPE] = 7.0;
    actionWeights[ActionTypes::CHILL] = 5.0;
    actionWeights[ActionTypes::WIPEOUT] = 5.0;
}

Player_PlanEvolution::Player_PlanEvolution(const PlayerID playerID, const PlanEvolutionParameters & params)
    : _params(params.clone())
    , _rngState(1469598103934665603ULL + (unsigned long long)playerID * 1099511628211ULL)
    , _lastGenerations(0)
    , _lastPlansEvaluated(0)
    , _lastBestScore(0)
    , _lastTimeMS(0)
{
    m_playerID = playerID;

    PRISMATA_ASSERT(m_playerID == _params.maxPlayer(), "PlanEvolution parameters do not match PlayerID");
    PRISMATA_ASSERT(_params.populationSize() > 0, "PlanEvolution population must be positive");
    PRISMATA_ASSERT(_params.eliteSize() > 0, "PlanEvolution elite size must be positive");
    PRISMATA_ASSERT(_params.eliteSize() <= _params.populationSize(), "PlanEvolution elite size must be <= population");
}

bool Player_PlanEvolution::searchTimeOut(Timer & timer) const
{
    return _params.timeLimit() > 0 && timer.getElapsedTimeInMilliSec() >= _params.timeLimit();
}

unsigned int Player_PlanEvolution::randomInt()
{
    _rngState ^= _rngState << 13;
    _rngState ^= _rngState >> 7;
    _rngState ^= _rngState << 17;

    return (unsigned int)(_rngState >> 32);
}

size_t Player_PlanEvolution::randomIndex(const size_t size)
{
    PRISMATA_ASSERT(size > 0, "Cannot choose random index from empty range");

    return (size_t)(randomInt() % size);
}

double Player_PlanEvolution::randomDouble()
{
    return (double)randomInt() / (double)std::numeric_limits<unsigned int>::max();
}

double Player_PlanEvolution::randomSigned()
{
    return randomDouble() * 2.0 - 1.0;
}

double Player_PlanEvolution::randomRange(const double low, const double high)
{
    return low + (high - low) * randomDouble();
}

void Player_PlanEvolution::mutateValue(double & value, const double low, const double high)
{
    if (randomDouble() <= _params.mutationRate())
    {
        value = ClampDouble(value + randomSigned() * _params.mutationStrength(), low, high);
    }
}

Player_PlanEvolution::Plan Player_PlanEvolution::makeBalancedPlan(const GameState & state)
{
    Plan plan;
    plan.buyWeights = std::vector<double>(CardTypes::GetAllCardTypes().size(), 1.0);

    for (CardID c(0); c < state.numCardsBuyable(); ++c)
    {
        const CardType type = state.getCardBuyableByIndex(c).getType();
        plan.buyWeights[type.getID()] = 1.0 + 0.05 * HeuristicValues::Instance().GetInflatedTotalCostValue(type);
    }

    return plan;
}

Player_PlanEvolution::Plan Player_PlanEvolution::makeSeedPlan(const GameState & state, const size_t seedIndex)
{
    Plan plan = makeBalancedPlan(state);

    if (seedIndex == 1)
    {
        plan.econBias = 3.0;
        plan.attackBias = 0.7;
        plan.defenseBias = 1.2;
        plan.actionWeights[ActionTypes::BUY] = 7.0;
    }
    else if (seedIndex == 2)
    {
        plan.econBias = 0.7;
        plan.attackBias = 3.0;
        plan.defenseBias = 0.8;
        plan.actionWeights[ActionTypes::USE_ABILITY] = 8.0;
        plan.actionWeights[ActionTypes::ASSIGN_BREACH] = 9.0;
        plan.actionWeights[ActionTypes::ASSIGN_FRONTLINE] = 7.0;
    }
    else if (seedIndex == 3)
    {
        plan.econBias = 0.8;
        plan.attackBias = 0.7;
        plan.defenseBias = 4.0;
        plan.actionWeights[ActionTypes::ASSIGN_BLOCKER] = 10.0;
        plan.actionWeights[ActionTypes::BUY] = 6.0;
    }
    else if (seedIndex == 4)
    {
        plan.spendBias = 2.5;
        plan.actionWeights[ActionTypes::BUY] = 8.0;
        plan.endActionBias = -4.0;
        plan.endActionGrowth = 0.25;
    }
    else if (seedIndex == 5)
    {
        plan.endActionBias = 2.0;
        plan.endActionGrowth = 0.8;
        plan.actionWeights[ActionTypes::BUY] = 2.0;
        plan.actionWeights[ActionTypes::USE_ABILITY] = 3.0;
    }

    return plan;
}

Player_PlanEvolution::Plan Player_PlanEvolution::makeRandomPlan(const GameState & state)
{
    Plan plan = makeBalancedPlan(state);

    for (size_t i(0); i < ActionTypes::NUM_TYPES; ++i)
    {
        plan.actionWeights[i] = randomRange(-2.0, 10.0);
    }

    plan.actionWeights[ActionTypes::UNDO_USE_ABILITY] = randomRange(-8.0, -1.0);
    plan.actionWeights[ActionTypes::UNDO_CHILL] = randomRange(-8.0, -1.0);
    plan.actionWeights[ActionTypes::UNDO_BREACH] = randomRange(-8.0, -1.0);
    plan.actionWeights[ActionTypes::SELL] = randomRange(-8.0, -1.0);

    plan.econBias = randomRange(0.0, 4.0);
    plan.attackBias = randomRange(0.0, 4.0);
    plan.defenseBias = randomRange(0.0, 4.0);
    plan.frontlineBias = randomRange(0.0, 4.0);
    plan.targetBias = randomRange(0.0, 4.0);
    plan.spendBias = randomRange(0.0, 3.0);
    plan.endActionBias = randomRange(-5.0, 3.0);
    plan.endActionGrowth = randomRange(0.05, 1.5);

    for (size_t i(0); i < plan.buyWeights.size(); ++i)
    {
        plan.buyWeights[i] = randomRange(0.0, 5.0);
    }

    return plan;
}

std::vector<Player_PlanEvolution::Plan> Player_PlanEvolution::makeInitialPopulation(const GameState & state)
{
    std::vector<Plan> population;
    population.reserve(_params.populationSize());

    for (size_t i(0); i < _params.populationSize(); ++i)
    {
        if (i < 6)
        {
            population.push_back(makeSeedPlan(state, i));
        }
        else
        {
            population.push_back(makeRandomPlan(state));
        }
    }

    return population;
}

Player_PlanEvolution::Plan Player_PlanEvolution::crossover(const Plan & a, const Plan & b)
{
    Plan child = a;

    for (size_t i(0); i < ActionTypes::NUM_TYPES; ++i)
    {
        child.actionWeights[i] = randomDouble() < 0.5 ? a.actionWeights[i] : b.actionWeights[i];
    }

    child.econBias = randomDouble() < 0.5 ? a.econBias : b.econBias;
    child.attackBias = randomDouble() < 0.5 ? a.attackBias : b.attackBias;
    child.defenseBias = randomDouble() < 0.5 ? a.defenseBias : b.defenseBias;
    child.frontlineBias = randomDouble() < 0.5 ? a.frontlineBias : b.frontlineBias;
    child.targetBias = randomDouble() < 0.5 ? a.targetBias : b.targetBias;
    child.spendBias = randomDouble() < 0.5 ? a.spendBias : b.spendBias;
    child.endActionBias = randomDouble() < 0.5 ? a.endActionBias : b.endActionBias;
    child.endActionGrowth = randomDouble() < 0.5 ? a.endActionGrowth : b.endActionGrowth;

    for (size_t i(0); i < child.buyWeights.size(); ++i)
    {
        child.buyWeights[i] = randomDouble() < 0.5 ? a.buyWeights[i] : b.buyWeights[i];
    }

    child.evaluated = false;
    child.score = -std::numeric_limits<double>::max();
    child.move.clear();

    return child;
}

void Player_PlanEvolution::mutate(Plan & plan)
{
    for (size_t i(0); i < ActionTypes::NUM_TYPES; ++i)
    {
        mutateValue(plan.actionWeights[i], -10.0, 12.0);
    }

    mutateValue(plan.econBias, 0.0, 6.0);
    mutateValue(plan.attackBias, 0.0, 6.0);
    mutateValue(plan.defenseBias, 0.0, 6.0);
    mutateValue(plan.frontlineBias, 0.0, 6.0);
    mutateValue(plan.targetBias, 0.0, 6.0);
    mutateValue(plan.spendBias, 0.0, 5.0);
    mutateValue(plan.endActionBias, -8.0, 5.0);
    mutateValue(plan.endActionGrowth, 0.0, 2.0);

    for (size_t i(0); i < plan.buyWeights.size(); ++i)
    {
        mutateValue(plan.buyWeights[i], 0.0, 8.0);
    }

    plan.actionWeights[ActionTypes::UNDO_USE_ABILITY] = std::min(plan.actionWeights[ActionTypes::UNDO_USE_ABILITY], -1.0);
    plan.actionWeights[ActionTypes::UNDO_CHILL] = std::min(plan.actionWeights[ActionTypes::UNDO_CHILL], -1.0);
    plan.actionWeights[ActionTypes::UNDO_BREACH] = std::min(plan.actionWeights[ActionTypes::UNDO_BREACH], -1.0);
    plan.evaluated = false;
}

double Player_PlanEvolution::evaluateState(const GameState & state)
{
    if (state.isGameOver())
    {
        if (state.winner() == _params.maxPlayer())
        {
            return Eval::WinScore;
        }

        if (state.winner() == state.getEnemy(_params.maxPlayer()))
        {
            return -Eval::WinScore;
        }

        return 0;
    }

    if (_params.evalMethod() == EvaluationMethods::Playout)
    {
        return Eval::ABPlayoutScore(state, _params.getPlayoutPlayer(Players::Player_One), _params.getPlayoutPlayer(Players::Player_Two), _params.maxPlayer());
    }

    if (_params.evalMethod() == EvaluationMethods::WillScore)
    {
        return Eval::WillScoreEvaluation(state, _params.maxPlayer());
    }

    return Eval::WillScoreInflationEvaluation(state, _params.maxPlayer());
}

double Player_PlanEvolution::evaluatePlan(const GameState & state, Plan & plan)
{
    GameState child(state);

    plan.move.clear();
    compilePlan(state, plan, plan.move);
    child.doMove(plan.move);

    plan.score = evaluateState(child);
    plan.evaluated = true;

    return plan.score;
}

void Player_PlanEvolution::evaluatePopulation(const GameState & state, Timer & timer, std::vector<Plan> & population)
{
    for (size_t i(0); i < population.size(); ++i)
    {
        if (searchTimeOut(timer))
        {
            return;
        }

        if (!population[i].evaluated)
        {
            evaluatePlan(state, population[i]);
            _lastPlansEvaluated++;
        }
    }
}

void Player_PlanEvolution::addEndPhaseIfLegal(GameState & state, std::vector<Action> & actions) const
{
    const Action endPhase(state.getActivePlayer(), ActionTypes::END_PHASE, 0);

    if (state.isLegal(endPhase))
    {
        for (size_t a(0); a < actions.size(); ++a)
        {
            if (actions[a] == endPhase)
            {
                return;
            }
        }

        actions.push_back(endPhase);
    }
}

double Player_PlanEvolution::scoreAction(const GameState & state, const Plan & plan, const Action & action, const size_t phaseActions) const
{
    const PlayerID player = state.getActivePlayer();
    const PlayerID enemy = state.getEnemy(player);
    const ActionID type = action.getType();

    double score = plan.actionWeights[type];

    switch (type)
    {
        case ActionTypes::END_PHASE:
        {
            if (state.getActivePhase() == Phases::Action)
            {
                return plan.endActionBias + (double)phaseActions * plan.endActionGrowth;
            }

            return 1000.0;
        }
        case ActionTypes::BUY:
        {
            const CardType cardType = state.getCardBuyableByID(action.getID()).getType();
            const double buyWeight = cardType.getID() < plan.buyWeights.size() ? plan.buyWeights[cardType.getID()] : 1.0;
            const double cost = HeuristicValues::Instance().GetBuyTotalCost(cardType);
            const double attack = Heuristics::GetAttackProduced(cardType, state, player);
            const double defense = cardType.canBlock(false) ? cardType.getStartingHealth() : 0;

            score += 2.0 * buyWeight;
            score += plan.spendBias * cost;
            score += plan.econBias * (cardType.isEconCard() ? 4.0 : 0.0);
            score += plan.attackBias * (attack + cardType.getAttack());
            score += plan.defenseBias * defense;
            score += plan.frontlineBias * (cardType.isFrontline() ? 3.0 : 0.0);
            score -= plan.defenseBias * cardType.getAttackGivenToEnemy();
            break;
        }
        case ActionTypes::USE_ABILITY:
        {
            const Card & card = state.getCardByID(action.getID());
            const CardType cardType = card.getType();
            const double attack = Heuristics::GetAttackProduced(card, state, player);

            score += plan.attackBias * attack;
            score += plan.econBias * (cardType.isEconCard() ? 2.0 : 0.0);
            score += plan.defenseBias * cardType.getHealthGained();

            if (cardType.hasTargetAbility())
            {
                GameState targetState(state);
                std::vector<Action> targetActions;
                bool hasTarget = false;

                targetState.doAction(action);
                targetState.generateLegalActions(targetActions);

                for (size_t a(0); a < targetActions.size(); ++a)
                {
                    if (targetActions[a].getType() == cardType.getTargetAbilityType())
                    {
                        hasTarget = true;
                        break;
                    }
                }

                if (!hasTarget)
                {
                    return MinActionScore;
                }

                score += plan.targetBias * cardType.getTargetAbilityAmount();
            }

            break;
        }
        case ActionTypes::SNIPE:
        case ActionTypes::CHILL:
        {
            const Card & target = state.getCardByID(action.getTargetID());
            score += plan.targetBias * Heuristics::CurrentCardValue(target, state);

            if (type == ActionTypes::SNIPE)
            {
                score += plan.attackBias * target.currentHealth();
            }
            else
            {
                score += plan.defenseBias * target.getType().getAttack();
                score += plan.attackBias * Heuristics::GetAttackProduced(target, state, enemy);
            }

            break;
        }
        case ActionTypes::ASSIGN_BLOCKER:
        {
            const Card & card = state.getCardByID(action.getID());
            const double cardValue = Heuristics::CurrentCardValue(card, state);
            const double blockAmount = std::min<double>(card.currentHealth(), state.getAttack(enemy));
            const double attackLost = Heuristics::GetAttackProduced(card, state, player);

            score += plan.defenseBias * blockAmount * 3.0;
            score -= cardValue * 0.25;
            score -= plan.attackBias * attackLost;
            break;
        }
        case ActionTypes::ASSIGN_BREACH:
        case ActionTypes::ASSIGN_FRONTLINE:
        case ActionTypes::WIPEOUT:
        {
            const Card & target = state.getCardByID(action.getID());
            score += plan.targetBias * Heuristics::CurrentCardValue(target, state);
            score += plan.attackBias * target.currentHealth();

            if (type == ActionTypes::ASSIGN_FRONTLINE)
            {
                score += plan.frontlineBias * 3.0;
            }

            break;
        }
        case ActionTypes::SELL:
        case ActionTypes::UNDO_USE_ABILITY:
        case ActionTypes::UNDO_CHILL:
        case ActionTypes::UNDO_BREACH:
        {
            score -= 20.0;
            break;
        }
    }

    return score;
}

void Player_PlanEvolution::compilePlan(const GameState & state, const Plan & plan, Move & move, std::string * description)
{
    GameState current(state);
    move.clear();

    if (description)
    {
        description->clear();
    }

    size_t phaseActions = 0;
    int previousPhase = current.getActivePhase();

    while (!current.isGameOver() && current.getActivePlayer() == m_playerID && move.size() < _params.maxActions())
    {
        if (current.getActivePhase() != previousPhase)
        {
            previousPhase = current.getActivePhase();
            phaseActions = 0;
        }

        std::vector<Action> actions;
        current.generateLegalActions(actions);
        addEndPhaseIfLegal(current, actions);

        PRISMATA_ASSERT(!actions.empty(), "PlanEvolution compiler found no legal actions");

        size_t bestAction = 0;
        double bestScore = MinActionScore;

        for (size_t a(0); a < actions.size(); ++a)
        {
            const double actionScore = scoreAction(current, plan, actions[a], phaseActions);

            if (actionScore > bestScore)
            {
                bestScore = actionScore;
                bestAction = a;
            }
        }

        const Action action = actions[bestAction];

        if (description && description->size() < 2000)
        {
            *description += actionSummary(current, action);
            *description += "\n";
        }

        current.doAction(action);
        move.addAction(action);
        phaseActions++;

        if (action.getType() == ActionTypes::END_PHASE)
        {
            phaseActions = 0;
        }
    }
}

std::string Player_PlanEvolution::actionSummary(const GameState & state, const Action & action) const
{
    std::stringstream ss;

    if (action.getType() == ActionTypes::BUY)
    {
        ss << "BUY " << state.getCardBuyableByID(action.getID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::USE_ABILITY)
    {
        ss << "ABILITY " << state.getCardByID(action.getID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::ASSIGN_BLOCKER)
    {
        ss << "BLOCK " << state.getCardByID(action.getID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::ASSIGN_BREACH)
    {
        ss << "BREACH " << state.getCardByID(action.getID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::ASSIGN_FRONTLINE)
    {
        ss << "FRONTLINE " << state.getCardByID(action.getID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::SNIPE)
    {
        ss << "SNIPE " << state.getCardByID(action.getTargetID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::CHILL)
    {
        ss << "CHILL " << state.getCardByID(action.getTargetID()).getType().getUIName();
    }
    else if (action.getType() == ActionTypes::END_PHASE)
    {
        ss << "END_PHASE";
    }
    else
    {
        ss << action.toStringEnglishShort();
    }

    return ss.str();
}

std::string Player_PlanEvolution::getPlanDescription(const GameState & state, const Plan & plan) const
{
    std::stringstream ss;
    std::string actionDescription;
    Move ignored;

    const_cast<Player_PlanEvolution *>(this)->compilePlan(state, plan, ignored, &actionDescription);

    ss << "Plan score:     " << plan.score << "\n";
    ss << "Econ bias:      " << plan.econBias << "\n";
    ss << "Attack bias:    " << plan.attackBias << "\n";
    ss << "Defense bias:   " << plan.defenseBias << "\n";
    ss << "Spend bias:     " << plan.spendBias << "\n";
    ss << "Plan actions:\n" << actionDescription;

    return ss.str();
}

void Player_PlanEvolution::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    Timer timer;
    timer.start();

    _rngState ^= ((unsigned long long)state.getTurnNumber() + 1ULL) * 1099511628211ULL;
    _rngState ^= ((unsigned long long)state.numCards(m_playerID) + 17ULL) << 32;
    _lastGenerations = 0;
    _lastPlansEvaluated = 0;
    _lastBestScore = -std::numeric_limits<double>::max();
    _lastBestDescription.clear();

    std::vector<Plan> population = makeInitialPopulation(state);

    evaluatePopulation(state, timer, population);
    std::sort(population.begin(), population.end(), PlanScoreCompare);

    while (!searchTimeOut(timer) && (_params.maxGenerations() == 0 || _lastGenerations < _params.maxGenerations()))
    {
        const size_t eliteCount = std::min(_params.eliteSize(), population.size());

        std::vector<Plan> nextPopulation;
        nextPopulation.reserve(_params.populationSize());

        for (size_t e(0); e < eliteCount; ++e)
        {
            nextPopulation.push_back(population[e]);
        }

        while (nextPopulation.size() < _params.populationSize())
        {
            const Plan & a = population[randomIndex(eliteCount)];
            const Plan & b = population[randomIndex(eliteCount)];

            Plan child = crossover(a, b);
            mutate(child);
            nextPopulation.push_back(child);
        }

        population.swap(nextPopulation);
        evaluatePopulation(state, timer, population);
        std::sort(population.begin(), population.end(), PlanScoreCompare);

        _lastGenerations++;
    }

    if (!population.empty() && population.front().evaluated)
    {
        move = population.front().move;
        _lastBestScore = population.front().score;
        _lastBestDescription = getPlanDescription(state, population.front());
    }
    else
    {
        Plan fallback = makeBalancedPlan(state);
        compilePlan(state, fallback, move);
        _lastBestScore = 0;
    }

    _lastTimeMS = timer.getElapsedTimeInMilliSec();
}

PlanEvolutionParameters & Player_PlanEvolution::getParams()
{
    return _params;
}

std::string Player_PlanEvolution::getDescription()
{
    std::stringstream ss;

    ss << m_description << "\n";
    ss << "Plan Evolution\n";
    ss << "Time:           " << _lastTimeMS << "ms\n";
    ss << "Generations:    " << _lastGenerations << "\n";
    ss << "Plans Scored:   " << _lastPlansEvaluated << "\n";
    ss << "Best Score:     " << _lastBestScore << "\n";
    ss << _lastBestDescription;

    return ss.str();
}

PlayerPtr Player_PlanEvolution::clone()
{
    PlayerPtr ret(new Player_PlanEvolution(m_playerID, _params));
    ret->setDescription(m_description);

    return ret;
}
