#include "Player_PortfolioGreedySearch.h"
#include "Game.h"
#include "Player_PPSequence.h"

using namespace Prismata;

namespace
{
    const double PGSWinScore = 10000;
}

Player_PortfolioGreedySearch::Player_PortfolioGreedySearch(const PlayerID playerID, const double timeLimitMS, const size_t improvementIterations, const size_t responses, const MoveIteratorPtr & p1Portfolio, const MoveIteratorPtr & p2Portfolio)
    : _timeLimitMS(timeLimitMS)
    , _improvementIterations(improvementIterations)
    , _responses(responses)
{
    m_playerID = playerID;
    _portfolioIterators[Players::Player_One] = p1Portfolio;
    _portfolioIterators[Players::Player_Two] = p2Portfolio;
}

MoveIterator_PPPortfolio & Player_PortfolioGreedySearch::portfolio(const PlayerID player) const
{
    MoveIterator_PPPortfolio * portfolio = dynamic_cast<MoveIterator_PPPortfolio *>(_portfolioIterators[player].get());
    PRISMATA_ASSERT(portfolio != NULL, "Portfolio Greedy Search requires a PPPortfolio move iterator");

    return *portfolio;
}

Player_PortfolioGreedySearch::PortfolioSequence Player_PortfolioGreedySearch::getDefaultSequence(const PlayerID player) const
{
    PortfolioSequence sequence(PPPhases::NUM_PHASES, 0);

    for (size_t phase(0); phase < PPPhases::NUM_PHASES; ++phase)
    {
        PRISMATA_ASSERT(portfolio(player).numPartialPlayers(phase) > 0, "Portfolio phase has no partial players");
    }

    return sequence;
}

bool Player_PortfolioGreedySearch::timeExpired()
{
    return _timeLimitMS > 0 && _timer.getElapsedTimeInMilliSec() >= _timeLimitMS;
}

double Player_PortfolioGreedySearch::evaluate(const GameState & state, const PlayerID player, const PortfolioSequence & selfSequence, const PortfolioSequence & enemySequence)
{
    PortfolioSequence sequences[2];
    sequences[player] = selfSequence;
    sequences[state.getEnemy(player)] = enemySequence;

    PlayerPtr p1(new Player_PPSequence(Players::Player_One, portfolio(Players::Player_One).getSequence(sequences[Players::Player_One])));
    PlayerPtr p2(new Player_PPSequence(Players::Player_Two, portfolio(Players::Player_Two).getSequence(sequences[Players::Player_Two])));

    Game game(state, p1, p2);
    game.play();

    const PlayerID winner = game.getState().winner();
    if (winner == player)
    {
        return PGSWinScore - game.getTurnsPlayed();
    }
    else if (winner == state.getEnemy(player))
    {
        return -PGSWinScore + game.getTurnsPlayed();
    }

    return 0;
}

Player_PortfolioGreedySearch::PortfolioSequence Player_PortfolioGreedySearch::getSeedSequence(const GameState & state, const PlayerID player, const PortfolioSequence & enemySequence)
{
    return improve(state, player, getDefaultSequence(player), enemySequence, 1);
}

Player_PortfolioGreedySearch::PortfolioSequence Player_PortfolioGreedySearch::improve(const GameState & state, const PlayerID player, PortfolioSequence selfSequence, const PortfolioSequence & enemySequence, const size_t iterations)
{
    for (size_t iteration(0); iteration < iterations && !timeExpired(); ++iteration)
    {
        for (size_t phase(0); phase < PPPhases::NUM_PHASES && !timeExpired(); ++phase)
        {
            double bestValue = std::numeric_limits<double>::lowest();
            size_t bestIndex = selfSequence[phase];

            for (size_t candidateIndex(0); candidateIndex < portfolio(player).numPartialPlayers(phase); ++candidateIndex)
            {
                if (timeExpired())
                {
                    break;
                }

                PortfolioSequence candidateSequence(selfSequence);
                candidateSequence[phase] = candidateIndex;

                const double value = evaluate(state, player, candidateSequence, enemySequence);
                if (value > bestValue)
                {
                    bestValue = value;
                    bestIndex = candidateIndex;
                }
            }

            selfSequence[phase] = bestIndex;
        }
    }

    return selfSequence;
}

void Player_PortfolioGreedySearch::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _timer.start();

    const PlayerID enemy = state.getEnemy(m_playerID);
    PortfolioSequence enemySequence = getDefaultSequence(enemy);
    PortfolioSequence selfSequence = getSeedSequence(state, m_playerID, enemySequence);

    if (!timeExpired())
    {
        enemySequence = getSeedSequence(state, enemy, selfSequence);
    }

    if (!timeExpired())
    {
        selfSequence = improve(state, m_playerID, selfSequence, enemySequence, _improvementIterations);
    }

    for (size_t response(0); response < _responses && !timeExpired(); ++response)
    {
        enemySequence = improve(state, enemy, enemySequence, selfSequence, _improvementIterations);

        if (!timeExpired())
        {
            selfSequence = improve(state, m_playerID, selfSequence, enemySequence, _improvementIterations);
        }
    }

    Player_PPSequence sequencePlayer(m_playerID, portfolio(m_playerID).getSequence(selfSequence));
    sequencePlayer.getMove(state, move);
}

std::string Player_PortfolioGreedySearch::getDescription()
{
    std::stringstream ss;
    ss << m_description << "\n";
    ss << "Portfolio Greedy Search\n";
    ss << "Time Limit:     " << _timeLimitMS << "ms\n";
    ss << "Iterations:     " << _improvementIterations << "\n";
    ss << "Responses:      " << _responses;

    return ss.str();
}

PlayerPtr Player_PortfolioGreedySearch::clone()
{
    PlayerPtr ret(new Player_PortfolioGreedySearch(m_playerID, _timeLimitMS, _improvementIterations, _responses, _portfolioIterators[Players::Player_One]->clone(), _portfolioIterators[Players::Player_Two]->clone()));
    ret->setDescription(m_description);

    return ret;
}
