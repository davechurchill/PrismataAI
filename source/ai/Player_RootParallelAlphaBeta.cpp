#include "Player_RootParallelAlphaBeta.h"
#include "Eval.h"
#include "Game.h"

#include <future>

using namespace Prismata;

Player_RootParallelAlphaBeta::Player_RootParallelAlphaBeta(const PlayerID playerID, const AlphaBetaSearchParameters & params)
    : _params(params)
{
    m_playerID = playerID;
    PRISMATA_ASSERT(m_playerID == params.maxPlayer(), "RootParallelAlphaBeta parameters do not match PlayerID");
}

bool Player_RootParallelAlphaBeta::moveIsLegal(const GameState & state, const Move & move) const
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

double Player_RootParallelAlphaBeta::evaluate(const GameState & state) const
{
    if (_params.evalMethod() == EvaluationMethods::Playout)
    {
        Game game(state,
                  _params.getPlayoutPlayer(Players::Player_One)->clone(),
                  _params.getPlayoutPlayer(Players::Player_Two)->clone());

        while (!game.gameOver())
        {
            if (!game.playNextTurn(false))
            {
                break;
            }
        }

        if (game.getState().winner() == _params.maxPlayer())
        {
            return Eval::WinScore - game.getTurnsPlayed();
        }
        else if (game.getState().winner() == state.getEnemy(_params.maxPlayer()))
        {
            return -Eval::WinScore + game.getTurnsPlayed();
        }

        return 0;
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScore)
    {
        return Eval::WillScoreEvaluation(state, _params.maxPlayer());
    }
    else if (_params.evalMethod() == EvaluationMethods::WillScoreInflation)
    {
        return Eval::WillScoreInflationEvaluation(state, _params.maxPlayer());
    }

    PRISMATA_ASSERT(false, "Unknown evaluation method");
    return 0;
}

double Player_RootParallelAlphaBeta::alphaBeta(const GameState & state,
                                               const size_t depth,
                                               const size_t maxDepth,
                                               double alpha,
                                               double beta,
                                               Timer & timer,
                                               bool & timedOut) const
{
    if (_params.timeLimit() > 0 && timer.getElapsedTimeInMilliSec() >= _params.timeLimit())
    {
        timedOut = true;
        return evaluate(state);
    }

    if (depth >= maxDepth || state.isGameOver())
    {
        return evaluate(state);
    }

    const PlayerID player = state.getActivePlayer();
    MoveIteratorPtr iterator = _params.getMoveIterator(player)->clone();
    iterator->setState(state);

    GameState child;
    Move childMove;
    size_t children = 0;
    bool generatedChild = false;

    if (player == _params.maxPlayer())
    {
        double bestValue = -std::numeric_limits<double>::max();

        while ((_params.maxChildren() == 0 || children < _params.maxChildren())
            && iterator->generateNextChild(child, childMove))
        {
            generatedChild = true;
            bestValue = std::max(bestValue, alphaBeta(child, depth + 1, maxDepth, alpha, beta, timer, timedOut));
            alpha = std::max(alpha, bestValue);
            children++;

            if (timedOut || alpha >= beta)
            {
                break;
            }
        }

        return generatedChild ? bestValue : evaluate(state);
    }

    double bestValue = std::numeric_limits<double>::max();

    while ((_params.maxChildren() == 0 || children < _params.maxChildren())
        && iterator->generateNextChild(child, childMove))
    {
        generatedChild = true;
        bestValue = std::min(bestValue, alphaBeta(child, depth + 1, maxDepth, alpha, beta, timer, timedOut));
        beta = std::min(beta, bestValue);
        children++;

        if (timedOut || alpha >= beta)
        {
            break;
        }
    }

    return generatedChild ? bestValue : evaluate(state);
}

Player_RootParallelAlphaBeta::RootResult Player_RootParallelAlphaBeta::searchRootChild(const GameState & child, const Move & move) const
{
    RootResult result;
    result.move = move;
    result.value = evaluate(child);

    Timer timer;
    timer.start();

    for (size_t depth(1); _params.maxDepth() <= 0 || depth <= static_cast<size_t>(_params.maxDepth()); ++depth)
    {
        bool timedOut = false;
        const double value = alphaBeta(child,
                                       1,
                                       depth,
                                       -std::numeric_limits<double>::max(),
                                       std::numeric_limits<double>::max(),
                                       timer,
                                       timedOut);

        if (timedOut)
        {
            break;
        }

        result.value = value;

        if (_params.timeLimit() > 0 && timer.getElapsedTimeInMilliSec() >= _params.timeLimit())
        {
            break;
        }
    }

    return result;
}

void Player_RootParallelAlphaBeta::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    MoveIteratorPtr rootIterator = _params.getRootMoveIterator(m_playerID)->clone();
    rootIterator->setState(state);

    std::vector<std::future<RootResult>> futures;
    GameState child;
    Move rootMove;
    size_t rootChildren = 0;

    while ((_params.maxChildren() == 0 || rootChildren < _params.maxChildren())
        && rootIterator->generateNextChild(child, rootMove))
    {
        if (rootMove.size() == 0 || !moveIsLegal(state, rootMove))
        {
            continue;
        }

        futures.push_back(std::async(std::launch::async, [this, child, rootMove]()
        {
            return searchRootChild(child, rootMove);
        }));
        rootChildren++;
    }

    PRISMATA_ASSERT(!futures.empty(), "RootParallelAlphaBeta found no root children");

    RootResult best;
    for (std::future<RootResult> & future : futures)
    {
        RootResult result = future.get();
        if (result.value > best.value)
        {
            best = result;
        }
    }

    move = best.move;
}

std::string Player_RootParallelAlphaBeta::getDescription()
{
    std::stringstream ss;
    ss << m_description << "\n";
    ss << "Root Parallel Alpha-Beta\n";
    ss << _params.getDescription();
    return ss.str();
}

PlayerPtr Player_RootParallelAlphaBeta::clone()
{
    PlayerPtr ret(new Player_RootParallelAlphaBeta(m_playerID, _params.clone()));
    ret->setDescription(m_description);
    return ret;
}
