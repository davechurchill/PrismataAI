#include "Player_AlphaBeta.h"

using namespace Prismata;

Player_AlphaBeta::Player_AlphaBeta (const PlayerID playerID, const AlphaBetaSearchParameters & params)
    : _search(params)
    , _params(params)
{
    m_playerID = playerID;

    PRISMATA_ASSERT(m_playerID == params.maxPlayer(), "ALphaBeta Search parameters do not match PlayerID");
}

void Player_AlphaBeta::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _search = AlphaBetaSearch(_params);

    _search.doSearch(state, move);
}

AlphaBetaSearchParameters & Player_AlphaBeta::getParams()
{
    return _params;
}

AlphaBetaSearchResults & Player_AlphaBeta::getResults()
{
    return _search.getResults();
}