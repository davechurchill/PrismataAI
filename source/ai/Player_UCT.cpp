#include "Player_UCT.h"

using namespace Prismata;

Player_UCT::Player_UCT (const PlayerID playerID, const UCTSearchParameters & params)
    : _search(params)
    , _params(params)

{
    m_playerID = playerID;

    PRISMATA_ASSERT(m_playerID == _params.maxPlayer(), "UCT Search parameters do not match PlayerID");
}

void Player_UCT::getMove(const GameState & state, Move & move)
{
    PRISMATA_ASSERT(m_playerID == state.getActivePlayer(), "It is not this player's turn to move.");

    _search = UCTSearch(_params);

    _search.doSearch(state, move);
}

UCTSearchParameters & Player_UCT::getParams()
{
    return _params;
}

UCTSearchResults & Player_UCT::getResults()
{
    return _search.getResults();
}