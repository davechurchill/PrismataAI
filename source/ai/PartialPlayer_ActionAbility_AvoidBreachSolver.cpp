#include "PartialPlayer_ActionAbility_AvoidBreachSolver.h"
#include "AvoidBreachBuyIterator.h"
#include "AITools.h"

using namespace Prismata;

PartialPlayer_ActionAbility_AvoidBreachSolver::PartialPlayer_ActionAbility_AvoidBreachSolver(const PlayerID & playerID, const BreachIteratorParameters & params)
    : _params(params)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_ABILITY;
}

void PartialPlayer_ActionAbility_AvoidBreachSolver::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }
    
    Move solveMoves;
    AvoidBreachBuyIterator abbi(state, _params);
    abbi.solve();
    abbi.getMove(solveMoves);

    state.doMove(solveMoves);
    move.addMove(solveMoves);
}
