#pragma once

#include "Common.h"
#include "Player.h"
#include "UCTSearch.h"
#include "UCTSearchParameters.hpp"

namespace Prismata
{

class Player_UCT : public Player
{
    UCTSearch               _search;
    UCTSearchParameters     _params;

public:
    Player_UCT(const PlayerID playerID, const UCTSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    UCTSearchParameters & getParams();
    UCTSearchResults & getResults();

    virtual std::string getDescription() { return m_description + "\n" + _search.getDescription(); };
    PlayerPtr clone() { return PlayerPtr(new Player_UCT(*this)); }
};
}