#pragma once

#include "Common.h"
#include "Player.h"
#include "UCTSearch.h"
#include "UCTSearchParameters.hpp"

namespace Prismata
{

class Player_UCT : public Player
{
    UCTSearchParameters     _params;
    UCTSearch               _search;

public:
    Player_UCT(const PlayerID playerID, const UCTSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    UCTSearchParameters & getParams();
    UCTSearchResults & getResults();

    virtual std::string getDescription() { return m_description + "\n" + _search.getDescription(); };
    PlayerPtr clone() { PlayerPtr ret(new Player_UCT(m_playerID, _params)); ret->setDescription(m_description); return ret; }
};
}
