#include "Player_StackAlphaBeta.h"

using namespace Prismata;

Player_StackAlphaBeta::Player_StackAlphaBeta (const PlayerID & playerID, const AlphaBetaSearchParameters & params)
    : _search(params)
    , _params(params)
{
    m_playerID = playerID;

    PRISMATA_ASSERT(m_playerID == params.maxPlayer(), "ALphaBeta Search parameters do not match PlayerID");
}

void Player_StackAlphaBeta::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _search = StackAlphaBetaSearch(_params);

    _search.doSearch(state, move);
}

AlphaBetaSearchParameters & Player_StackAlphaBeta::getParams()
{
    return _params;
}

AlphaBetaSearchResults & Player_StackAlphaBeta::getResults()
{
    return _search.getResults();
}