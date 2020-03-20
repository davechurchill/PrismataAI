#pragma once

#include "Common.h"
#include "GameState.h"
#include "Player.h"
#include "Heuristics.h"

namespace Prismata
{
namespace Eval
{

namespace PlayoutTieBreaker
{
    enum { None, Turns, EndWillScore, Size};
}

    extern const double WinScore;

    PlayerID    PerformPlayout(const GameState & state, const PlayerPtr & p1, const PlayerPtr & p2);
    double      ABPlayoutScore(const GameState & state, const PlayerPtr & p1, const PlayerPtr & p2, const PlayerID maxPlayer);
    double      WillScoreSum(const GameState & state, const PlayerID player);
    double      WillScoreEvaluation(const GameState & state, const PlayerID maxPlayer);
    double      WillScoreInflationEvaluation(const GameState & state, const PlayerID maxPlayer);
    PlayerID    WillScoreEvalWinner(const GameState & state);
}
}
