#pragma once

#include "Common.h"
#include "LazyUCTSearch.h"
#include "LazyUCTSearchParameters.hpp"
#include "Player.h"

namespace Prismata
{

class Player_ProgressiveWideningUCT : public Player
{
    LazyUCTSearchParameters     _params;
    LazyUCTSearch               _search;

public:
    Player_ProgressiveWideningUCT(const PlayerID playerID, const LazyUCTSearchParameters & params);
    void getMove(const GameState & state, Move & move);
    LazyUCTSearchParameters & getParams();
    LazyUCTSearchResults & getResults();

    virtual std::string getDescription() { return m_description + "\n" + _search.getDescription(); }
    PlayerPtr clone() { PlayerPtr ret(new Player_ProgressiveWideningUCT(m_playerID, _params)); ret->setDescription(m_description); return ret; }
};

}
