#pragma once

#include "Common.h"
#include "Player.h"
#include "StackAlphaBetaSearch.h"

#include "AlphaBetaSearch.h"

namespace Prismata
{

class Player_StackAlphaBeta : public Player
{
    StackAlphaBetaSearch                    _search;
    AlphaBetaSearchParameters               _params;
public:
    Player_StackAlphaBeta(const PlayerID playerID, const AlphaBetaSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    AlphaBetaSearchParameters & getParams();
    AlphaBetaSearchResults & getResults();
    std::string getDescription() { return m_description + "\n" + _search.getDescription(); };

    PlayerPtr clone() { return PlayerPtr(new Player_StackAlphaBeta(*this)); }
};
}