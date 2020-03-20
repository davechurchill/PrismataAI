#include "Eval.h"
#include "Game.h"

namespace Prismata
{
namespace Eval
{
    const double WinScore = 10000;

    PlayerID PerformPlayout(const GameState & state, const PlayerPtr & p1, const PlayerPtr & p2)
    {
        Game g(state, p1, p2);
        g.play();
        return g.getState().winner();
    }

    double ABPlayoutScore(const GameState & state, const PlayerPtr & p1, const PlayerPtr & p2, const PlayerID maxPlayer)
    {
        Game g(state, p1, p2);
        g.play();

        if (g.getState().winner() == maxPlayer)
        {
            return WinScore - g.getTurnsPlayed();
        }
        else if (g.getState().winner() == state.getEnemy(maxPlayer))
        {
            return -WinScore + g.getTurnsPlayed();
        }

        return 0;
    }
    
    double WillScoreSum(const GameState & state, const PlayerID player)
    {
        double sum = 0;

        for (const auto & cardID : state.getCardIDs(player))
        {
            sum += Heuristics::CurrentCardValue(state.getCardByID(cardID), state);
        }

        return sum;
    }

    double WillScoreInflationEvaluation(const GameState & state, const PlayerID maxPlayer)
    {
        PlayerID player = state.getActivePlayer();

        double evalOne = Eval::WillScoreSum(state, 0) * (player == 0 ? 1.13 : 1.0);
        double evalTwo = Eval::WillScoreSum(state, 1) * (player == 1 ? 1.13 : 1.0);
        double diff = (maxPlayer == 0) ? (evalOne - evalTwo) : (evalTwo - evalOne);

        return diff;
    }

    double WillScoreEvaluation(const GameState & state, const PlayerID maxPlayer)
    {
        PlayerID player = state.getActivePlayer();

        double evalOne = Eval::WillScoreSum(state, 0);
        double evalTwo = Eval::WillScoreSum(state, 1);
        double diff = (maxPlayer == 0) ? (evalOne - evalTwo) : (evalTwo - evalOne);

        return diff;
    }

    PlayerID WillScoreEvalWinner(const GameState & state)
    {
        double score = WillScoreEvaluation(state, Players::Player_One);

        if (score > 0)
        {
            return Players::Player_One;
        }
        else if (score < 0)
        {
            return Players::Player_Two;
        }
        else
        {
            return Players::Player_None;
        }
    }
}
}

