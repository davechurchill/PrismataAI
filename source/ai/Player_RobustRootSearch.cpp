#include "Player_RobustRootSearch.h"
#include "Eval.h"
#include "Game.h"

using namespace Prismata;

Player_RobustRootSearch::Player_RobustRootSearch(const PlayerID playerID,
                                                 const Parameters & params,
                                                 const MoveIteratorPtr & p1RootIterator,
                                                 const MoveIteratorPtr & p2RootIterator,
                                                 const MoveIteratorPtr & p1ResponseIterator,
                                                 const MoveIteratorPtr & p2ResponseIterator,
                                                 const PlayerPtr & p1PlayoutPlayer,
                                                 const PlayerPtr & p2PlayoutPlayer)
    : _params(params)
{
    m_playerID = playerID;
    _rootIterators[Players::Player_One] = p1RootIterator;
    _rootIterators[Players::Player_Two] = p2RootIterator;
    _responseIterators[Players::Player_One] = p1ResponseIterator;
    _responseIterators[Players::Player_Two] = p2ResponseIterator;
    _playoutPlayers[Players::Player_One] = p1PlayoutPlayer;
    _playoutPlayers[Players::Player_Two] = p2PlayoutPlayer;
}

bool Player_RobustRootSearch::timeExpired()
{
    return _params.timeLimitMS > 0 && _timer.getElapsedTimeInMilliSec() >= _params.timeLimitMS;
}

bool Player_RobustRootSearch::moveIsLegal(const GameState & state, const Move & move) const
{
    GameState child(state);

    for (size_t a(0); a < move.size(); ++a)
    {
        if (!child.isLegal(move.getAction(a)))
        {
            return false;
        }

        child.doAction(move.getAction(a));
    }

    return true;
}

double Player_RobustRootSearch::evaluatePlayout(const GameState & state) const
{
    Game game(state,
              _playoutPlayers[Players::Player_One]->clone(),
              _playoutPlayers[Players::Player_Two]->clone());

    game.setTurnLimit(static_cast<TurnType>(_params.playoutTurnLimit));
    game.play();

    const PlayerID winner = game.getState().winner();
    if (winner == m_playerID)
    {
        return Eval::WinScore - game.getTurnsPlayed();
    }
    else if (winner == state.getEnemy(m_playerID))
    {
        return -Eval::WinScore + game.getTurnsPlayed();
    }

    return Eval::WillScoreEvaluation(game.getState(), m_playerID) * 0.01;
}

double Player_RobustRootSearch::combineScores(const double directScore, const double worstScore, const double averageScore, const size_t responses) const
{
    if (responses == 0)
    {
        return directScore;
    }

    return (_params.directWeight * directScore)
         + (_params.worstResponseWeight * worstScore)
         + (_params.averageResponseWeight * averageScore);
}

Player_RobustRootSearch::CandidateResult Player_RobustRootSearch::evaluateCandidate(const GameState & childState, const Move & rootMove)
{
    CandidateResult result;
    result.move = rootMove;
    result.directScore = evaluatePlayout(childState);
    result.worstResponseScore = result.directScore;
    result.averageResponseScore = result.directScore;
    result.score = result.directScore;

    if (timeExpired() || childState.isGameOver())
    {
        return result;
    }

    const PlayerID responsePlayer = childState.getActivePlayer();
    MoveIteratorPtr responseIterator = _responseIterators[responsePlayer]->clone();
    responseIterator->setState(childState);

    GameState responseChild;
    Move responseMove;
    double responseScoreSum = 0;
    double responseWorstScore = std::numeric_limits<double>::max();
    size_t responseCount = 0;

    while (!timeExpired()
        && (_params.maxResponseMoves == 0 || responseCount < _params.maxResponseMoves)
        && responseIterator->generateNextChild(responseChild, responseMove))
    {
        const double responseScore = evaluatePlayout(responseChild);
        responseScoreSum += responseScore;
        responseWorstScore = std::min(responseWorstScore, responseScore);
        responseCount++;
    }

    if (responseCount > 0)
    {
        result.responses = responseCount;
        result.worstResponseScore = responseWorstScore;
        result.averageResponseScore = responseScoreSum / responseCount;
        result.score = combineScores(result.directScore, result.worstResponseScore, result.averageResponseScore, result.responses);
    }

    return result;
}

void Player_RobustRootSearch::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _timer.start();

    MoveIteratorPtr rootIterator = _rootIterators[m_playerID]->clone();
    rootIterator->setState(state);

    GameState child;
    Move candidateMove;
    CandidateResult best;
    bool foundCandidate = false;
    size_t rootCount = 0;

    while (!timeExpired()
        && (_params.maxRootMoves == 0 || rootCount < _params.maxRootMoves)
        && rootIterator->generateNextChild(child, candidateMove))
    {
        if (candidateMove.size() == 0 || !moveIsLegal(state, candidateMove))
        {
            continue;
        }

        if (!foundCandidate)
        {
            best.move = candidateMove;
            foundCandidate = true;
        }

        CandidateResult result = evaluateCandidate(child, candidateMove);
        if (result.score > best.score)
        {
            best = result;
        }

        rootCount++;
    }

    PRISMATA_ASSERT(foundCandidate, "RobustRootSearch found no legal root candidates");
    move = best.move;
}

std::string Player_RobustRootSearch::getDescription()
{
    std::stringstream ss;
    ss << m_description << "\n";
    ss << "Robust Root Search\n";
    ss << "Time Limit:        " << _params.timeLimitMS << "ms\n";
    ss << "Max Root Moves:    " << _params.maxRootMoves << "\n";
    ss << "Max Responses:     " << _params.maxResponseMoves << "\n";
    ss << "Playout Turn Limit:" << _params.playoutTurnLimit << "\n";
    ss << "Direct Weight:     " << _params.directWeight << "\n";
    ss << "Worst Weight:      " << _params.worstResponseWeight << "\n";
    ss << "Average Weight:    " << _params.averageResponseWeight;

    return ss.str();
}

PlayerPtr Player_RobustRootSearch::clone()
{
    PlayerPtr ret(new Player_RobustRootSearch(m_playerID,
                                              _params,
                                              _rootIterators[Players::Player_One]->clone(),
                                              _rootIterators[Players::Player_Two]->clone(),
                                              _responseIterators[Players::Player_One]->clone(),
                                              _responseIterators[Players::Player_Two]->clone(),
                                              _playoutPlayers[Players::Player_One]->clone(),
                                              _playoutPlayers[Players::Player_Two]->clone()));
    ret->setDescription(m_description);

    return ret;
}
