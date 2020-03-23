#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class PartialPlayer_ActionBuy_AvoidBreach : public PartialPlayer
{

public:
    PartialPlayer_ActionBuy_AvoidBreach (const PlayerID playerID);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_AvoidBreach(*this));}
};

class CompareMostDefense 
{

public:

    CompareMostDefense()
    {
    }

    bool operator() (const CardType c1, const CardType c2) const
    {
        const double c1props[3] = { (double)(c1.isFragile() ? 0 : 1),      
                                    (double)-c1.getLifespan(),            
                                    (double)c1.getStartingHealth()};     

        const double c2props[3] = { (double)(c2.isFragile() ? 0 : 1),      
                                    (double)-c2.getLifespan(),            
                                    (double)c2.getStartingHealth()}; 

        for (size_t i(0); i < 3; ++i)
        {
            if (c1props[i] > c2props[i])
            {
                return true;
            }

            if (c1props[i] < c2props[i])
            {
                return false;
            }
        }

        return c1.getID() < c2.getID();
    }
};
}