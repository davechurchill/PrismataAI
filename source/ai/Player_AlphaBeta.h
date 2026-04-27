#pragma once

#include "Common.h"
#include "Player.h"
#include "StackAlphaBetaSearch.h"

#include "AlphaBetaSearch.h"

namespace Prismata
{

class Player_AlphaBeta : public Player
{
    AlphaBetaSearch                     _search;
    AlphaBetaSearchParameters           _params;
public:
    Player_AlphaBeta(const PlayerID playerID, const AlphaBetaSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    AlphaBetaSearchParameters & getParams();
    AlphaBetaSearchResults & getResults();
    std::string getDescription() { return m_description + "\n" + _search.getDescription(); };

    PlayerPtr clone() { return PlayerPtr(new Player_AlphaBeta(*this)); }
};
}