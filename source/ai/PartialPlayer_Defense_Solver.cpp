#include "PartialPlayer_Defense_Solver.h"

using namespace Prismata;

PartialPlayer_Defense_Solver::PartialPlayer_Defense_Solver(const PlayerID playerID, EvaluationType (*heuristic)(const Card &, const GameState & state, const HealthType &))
    : _heuristic(heuristic)
{
    _playerID = playerID;
    _phaseID = PPPhases::DEFENSE;
}

void PartialPlayer_Defense_Solver::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Defense)
    {
        return;
    }

    BlockIterator blockIterator(state, _heuristic);
    blockIterator.solve();
    blockIterator.getBestMove(move);

    for (size_t a(0); a<move.size(); ++a)
    {
        PRISMATA_ASSERT(state.isLegal(move.getAction(a)), "This move should have been legal!");

        state.doAction(move.getAction(a));
    }

    Action end(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    PRISMATA_ASSERT(state.isLegal(end), "Should be able to end defense phase");

    state.doAction(end);
    move.addAction(end);
}
