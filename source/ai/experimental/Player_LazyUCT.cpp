#include "Player_LazyUCT.h"

using namespace Prismata;

Player_LazyUCT::Player_LazyUCT(const PlayerID playerID, const LazyUCTSearchParameters & params)
    : _params(params.clone())
    , _search(_params)
{
    m_playerID = playerID;
    _params.setProgressiveWidening(false);

    PRISMATA_ASSERT(m_playerID == _params.maxPlayer(), "LazyUCT Search parameters do not match PlayerID");
}

void Player_LazyUCT::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _search = LazyUCTSearch(_params);
    _search.doSearch(state, move);
}

LazyUCTSearchParameters & Player_LazyUCT::getParams()
{
    return _params;
}

LazyUCTSearchResults & Player_LazyUCT::getResults()
{
    return _search.getResults();
}
