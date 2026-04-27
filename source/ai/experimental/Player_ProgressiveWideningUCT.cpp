#include "Player_ProgressiveWideningUCT.h"

using namespace Prismata;

Player_ProgressiveWideningUCT::Player_ProgressiveWideningUCT(const PlayerID playerID, const LazyUCTSearchParameters & params)
    : _params(params.clone())
    , _search(_params)
{
    m_playerID = playerID;
    _params.setProgressiveWidening(true);

    PRISMATA_ASSERT(m_playerID == _params.maxPlayer(), "ProgressiveWideningUCT Search parameters do not match PlayerID");
}

void Player_ProgressiveWideningUCT::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _search = LazyUCTSearch(_params);
    _search.doSearch(state, move);
}

LazyUCTSearchParameters & Player_ProgressiveWideningUCT::getParams()
{
    return _params;
}

LazyUCTSearchResults & Player_ProgressiveWideningUCT::getResults()
{
    return _search.getResults();
}
