#pragma once

#include "Common.h"
#include "MoveIterator.h"
#include "Player.h"
#include "Timer.h"

namespace Prismata
{

class Player_RobustRootSearch : public Player
{
public:
    struct Parameters
    {
        double timeLimitMS = 1000.0;
        size_t maxRootMoves = 0;
        size_t maxResponseMoves = 0;
        size_t playoutTurnLimit = 200;
        double directWeight = 0.0;
        double worstResponseWeight = 0.8;
        double averageResponseWeight = 0.2;
    };

private:
    struct CandidateResult
    {
        Move move;
        double score = -std::numeric_limits<double>::max();
        double directScore = 0;
        double worstResponseScore = 0;
        double averageResponseScore = 0;
        size_t responses = 0;
    };

    Parameters    _params;
    MoveIteratorPtr _rootIterators[2];
    MoveIteratorPtr _responseIterators[2];
    PlayerPtr     _playoutPlayers[2];
    Timer         _timer;

    bool timeExpired();
    bool moveIsLegal(const GameState & state, const Move & move) const;
    double evaluatePlayout(const GameState & state) const;
    CandidateResult evaluateCandidate(const GameState & childState, const Move & rootMove);
    double combineScores(const double directScore, const double worstScore, const double averageScore, const size_t responses) const;

public:
    Player_RobustRootSearch(const PlayerID playerID,
                            const Parameters & params,
                            const MoveIteratorPtr & p1RootIterator,
                            const MoveIteratorPtr & p2RootIterator,
                            const MoveIteratorPtr & p1ResponseIterator,
                            const MoveIteratorPtr & p2ResponseIterator,
                            const PlayerPtr & p1PlayoutPlayer,
                            const PlayerPtr & p2PlayoutPlayer);

    void getMove(const GameState & state, Move & move);
    std::string getDescription();
    PlayerPtr clone();
};

}
