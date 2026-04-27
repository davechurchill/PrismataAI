#include "Player_RootRacing.h"
#include "Eval.h"
#include "Timer.h"

#include <cmath>

using namespace Prismata;

namespace
{
    bool CandidateScoreCompare(const Player_RootRacing::Candidate & lhs, const Player_RootRacing::Candidate & rhs)
    {
        if ((lhs.visits > 0) != (rhs.visits > 0))
        {
            return lhs.visits > rhs.visits;
        }

        if (lhs.score() == rhs.score())
        {
            return lhs.seedScore > rhs.seedScore;
        }

        return lhs.score() > rhs.score();
    }
}

RootRacingParameters RootRacingParameters::clone() const
{
    RootRacingParameters params(*this);

    params._rootMoveIterators.clear();
    for (const auto & iterator : _rootMoveIterators)
    {
        params._rootMoveIterators.push_back(iterator->clone());
    }

    for (PlayerID p(0); p < 2; ++p)
    {
        params._playoutPlayers[p] = _playoutPlayers[p].get() ? _playoutPlayers[p]->clone() : PlayerPtr();
    }

    return params;
}

std::string RootRacingParameters::getDescription() const
{
    std::stringstream ss;

    ss << "Root Racing Parameters\n";
    ss << "Time Limit:     ";

    if (timeLimit() > 0)
    {
        ss << timeLimit() << "ms\n";
    }
    else
    {
        ss << "None\n";
    }

    ss << "Max Candidates: " << maxCandidates() << "\n";
    ss << "Race Passes:    " << maxRacePasses() << "\n";
    ss << "Keep Fraction:  " << keepFraction() << "\n";
    ss << "Round Robin:    " << (roundRobinRootIterators() ? "true" : "false") << "\n";
    ss << "Iterators:      " << getRootMoveIterators().size() << "\n";

    return ss.str();
}

Player_RootRacing::Player_RootRacing(const PlayerID playerID, const RootRacingParameters & params)
    : _params(params.clone())
{
    m_playerID = playerID;

    PRISMATA_ASSERT(m_playerID == _params.maxPlayer(), "RootRacing parameters do not match PlayerID");
    PRISMATA_ASSERT(!_params.getRootMoveIterators().empty(), "RootRacing requires at least one root move iterator");
}

bool Player_RootRacing::searchTimeOut(Timer & timer) const
{
    return _params.timeLimit() > 0 && timer.getElapsedTimeInMilliSec() >= _params.timeLimit();
}

bool Player_RootRacing::candidateExists(const std::vector<Candidate> & candidates, const Move & move, const GameState & state) const
{
    for (const Candidate & candidate : candidates)
    {
        if (candidate.move == move || candidate.state.isIsomorphic(state))
        {
            return true;
        }
    }

    return false;
}

double Player_RootRacing::evaluateState(const GameState & state, const int evalMethod)
{
    if (state.isGameOver())
    {
        if (state.winner() == _params.maxPlayer())
        {
            return Eval::WinScore;
        }
        else if (state.winner() == state.getEnemy(_params.maxPlayer()))
        {
            return -Eval::WinScore;
        }

        return 0;
    }

    if (evalMethod == EvaluationMethods::Playout)
    {
        PRISMATA_ASSERT(_params.getPlayoutPlayer(Players::Player_One).get() != NULL, "RootRacing has no Player One playout player");
        PRISMATA_ASSERT(_params.getPlayoutPlayer(Players::Player_Two).get() != NULL, "RootRacing has no Player Two playout player");

        return Eval::ABPlayoutScore(state,
                                    _params.getPlayoutPlayer(Players::Player_One),
                                    _params.getPlayoutPlayer(Players::Player_Two),
                                    _params.maxPlayer());
    }
    else if (evalMethod == EvaluationMethods::WillScore)
    {
        return Eval::WillScoreEvaluation(state, _params.maxPlayer());
    }
    else if (evalMethod == EvaluationMethods::WillScoreInflation)
    {
        return Eval::WillScoreInflationEvaluation(state, _params.maxPlayer());
    }

    PRISMATA_ASSERT(false, "Unknown RootRacing eval method");
    return 0;
}

void Player_RootRacing::generateCandidates(const GameState & state, Timer & timer, std::vector<Candidate> & candidates)
{
    _lastCandidatesGenerated = 0;
    _lastCandidatesAfterDedupe = 0;

    if (_params.roundRobinRootIterators())
    {
        generateCandidatesRoundRobin(state, timer, candidates);
    }
    else
    {
        generateCandidatesSequential(state, timer, candidates);
    }
}

void Player_RootRacing::generateCandidatesSequential(const GameState & state, Timer & timer, std::vector<Candidate> & candidates)
{
    for (const auto & rootIterator : _params.getRootMoveIterators())
    {
        if (searchTimeOut(timer) && !candidates.empty())
        {
            break;
        }

        MoveIteratorPtr iterator = rootIterator->clone();
        iterator->setState(state);

        while (_params.maxCandidates() == 0 || candidates.size() < _params.maxCandidates())
        {
            Move move;
            GameState child;

            if (!iterator->generateNextChild(child, move))
            {
                break;
            }

            _lastCandidatesGenerated++;

            if (!candidateExists(candidates, move, child))
            {
                Candidate candidate;
                candidate.move = move;
                candidate.state = child;
                candidate.description = iterator->getDescription();
                candidates.push_back(candidate);
                _lastCandidatesAfterDedupe = candidates.size();
            }

            if (searchTimeOut(timer))
            {
                break;
            }
        }
    }
}

void Player_RootRacing::generateCandidatesRoundRobin(const GameState & state, Timer & timer, std::vector<Candidate> & candidates)
{
    std::vector<MoveIteratorPtr> iterators;
    iterators.reserve(_params.getRootMoveIterators().size());

    for (const auto & rootIterator : _params.getRootMoveIterators())
    {
        iterators.push_back(rootIterator->clone());
        iterators.back()->setState(state);
    }

    std::vector<bool> exhausted(iterators.size(), false);
    size_t numExhausted = 0;
    size_t iteratorIndex = 0;

    while (numExhausted < iterators.size() && (_params.maxCandidates() == 0 || candidates.size() < _params.maxCandidates()))
    {
        if (exhausted[iteratorIndex])
        {
            iteratorIndex = (iteratorIndex + 1) % iterators.size();
            continue;
        }

        Move move;
        GameState child;

        if (!iterators[iteratorIndex]->generateNextChild(child, move))
        {
            exhausted[iteratorIndex] = true;
            numExhausted++;
        }
        else
        {
            _lastCandidatesGenerated++;

            if (!candidateExists(candidates, move, child))
            {
                Candidate candidate;
                candidate.move = move;
                candidate.state = child;
                candidate.description = iterators[iteratorIndex]->getDescription();
                candidates.push_back(candidate);
                _lastCandidatesAfterDedupe = candidates.size();
            }
        }

        iteratorIndex = (iteratorIndex + 1) % iterators.size();

        if (searchTimeOut(timer))
        {
            break;
        }
    }
}

void Player_RootRacing::seedCandidates(std::vector<Candidate> & candidates)
{
    for (Candidate & candidate : candidates)
    {
        candidate.seedScore = evaluateState(candidate.state, _params.seedEvalMethod());
    }

    std::sort(candidates.begin(), candidates.end(), CandidateScoreCompare);
}

size_t Player_RootRacing::keepCount(const size_t candidates) const
{
    if (candidates <= 1 || _params.keepFraction() >= 1.0)
    {
        return candidates;
    }

    size_t keep = (size_t)std::ceil((double)candidates * _params.keepFraction());
    return std::max((size_t)1, keep);
}

void Player_RootRacing::raceCandidates(Timer & timer, std::vector<Candidate> & candidates)
{
    _lastRacePasses = 0;
    _lastEvaluations = 0;

    if (candidates.empty())
    {
        return;
    }

    const size_t maxRacePasses = std::max((size_t)1, _params.maxRacePasses());
    for (size_t pass(0); pass < maxRacePasses; ++pass)
    {
        size_t evaluationsThisPass = 0;

        for (Candidate & candidate : candidates)
        {
            if (searchTimeOut(timer) && evaluationsThisPass > 0)
            {
                break;
            }

            candidate.scoreSum += evaluateState(candidate.state, _params.evalMethod());
            candidate.visits++;
            evaluationsThisPass++;
            _lastEvaluations++;

            if (searchTimeOut(timer))
            {
                break;
            }
        }

        if (evaluationsThisPass == 0)
        {
            break;
        }

        _lastRacePasses++;
        std::sort(candidates.begin(), candidates.end(), CandidateScoreCompare);

        const size_t keep = keepCount(candidates.size());
        if (keep < candidates.size())
        {
            candidates.resize(keep);
        }

        if (searchTimeOut(timer) || candidates.size() <= 1)
        {
            break;
        }
    }
}

void Player_RootRacing::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    move.clear();

    Timer timer;
    timer.start();

    std::vector<Candidate> candidates;
    candidates.reserve(_params.maxCandidates() == 0 ? 80 : _params.maxCandidates());

    generateCandidates(state, timer, candidates);

    if (candidates.empty())
    {
        MoveIteratorPtr iterator = _params.getRootMoveIterators().front()->clone();
        GameState child;
        iterator->setState(state);

        PRISMATA_ASSERT(iterator->generateNextChild(child, move), "RootRacing failed to generate any root moves");
        return;
    }

    seedCandidates(candidates);
    raceCandidates(timer, candidates);

    std::sort(candidates.begin(), candidates.end(), CandidateScoreCompare);

    move = candidates.front().move;
    _lastBestScore = candidates.front().score();
    _lastBestDescription = candidates.front().description;
    _lastTimeMS = timer.getElapsedTimeInMilliSec();
}

RootRacingParameters & Player_RootRacing::getParams()
{
    return _params;
}

std::string Player_RootRacing::getDescription()
{
    std::stringstream ss;

    ss << m_description << "\n";
    ss << _params.getDescription();
    ss << "Generated:      " << _lastCandidatesGenerated << "\n";
    ss << "Candidates:     " << _lastCandidatesAfterDedupe << "\n";
    ss << "Race Passes:    " << _lastRacePasses << "\n";
    ss << "Evaluations:    " << _lastEvaluations << "\n";
    ss << "Best Score:     " << _lastBestScore << "\n";
    ss << "Time:           " << _lastTimeMS << "ms\n";
    ss << _lastBestDescription;

    return ss.str();
}

PlayerPtr Player_RootRacing::clone()
{
    PlayerPtr ret(new Player_RootRacing(m_playerID, _params));
    ret->setDescription(m_description);

    return ret;
}
