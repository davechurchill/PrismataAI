#include "PartialPlayer_ActionAbility_ChillSolver.h"
#include "StateChillIterator.h"

using namespace Prismata;

PartialPlayer_ActionAbility_ChillSolver::PartialPlayer_ActionAbility_ChillSolver(const PlayerID playerID, const size_t maxIterations)
    : _maxIterations(maxIterations)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_ChillSolver::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    // if we don't have any attack, don't chill anything since it won't matter
    if (state.getAttack(_playerID) == 0)
    {
        return;
    }

    StateChillIterator chillIterator(state);
    chillIterator.solve(_maxIterations);

    const Move & bestMove = chillIterator.getBestMove();

    state.doMove(bestMove);
    move.addMove(bestMove);
}
