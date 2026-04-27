#pragma once

#include "Common.h"
#include "Player.h"
#include "StackAlphaBetaSearch.h"

#include "AlphaBetaSearch.h"

namespace Prismata
{

class Player_AlphaBeta : public Player
{
    AlphaBetaSearchParameters           _params;
    AlphaBetaSearch                     _search;
public:
    Player_AlphaBeta(const PlayerID playerID, const AlphaBetaSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    AlphaBetaSearchParameters & getParams();
    AlphaBetaSearchResults & getResults();
    std::string getDescription() { return m_description + "\n" + _search.getDescription(); };

    PlayerPtr clone() { PlayerPtr ret(new Player_AlphaBeta(m_playerID, _params)); ret->setDescription(m_description); return ret; }
};
}
