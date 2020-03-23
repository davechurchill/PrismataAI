#pragma once

#include "Common.h"
#include "CardType.h"

namespace Prismata
{

class CardBuyable
{
    CardType    m_type;                          // the card type represented
    SupplyType  m_maxSupply[2]       = {0, 0};   // the amount of supply of this card remaining for each player
    SupplyType  m_supplyRemaining[2] = {0, 0};   // the amount of supply of this card remaining for each player

public:

    CardBuyable();
    CardBuyable(const CardType type, const CardID p1MaxSupply, const CardID p2MaxSupply, const CardID p1Spent, const CardID p2Spent);

    CardBuyable & operator = (const CardBuyable & rhs); 
    
    bool operator < (const CardBuyable & rhs) const;

    const CardType getType() const;
    SupplyType  getSupplyRemaining(const PlayerID player) const;
    SupplyType  getMaxSupply(const PlayerID player) const;
    void        setSupplyRemaining(const PlayerID player, const SupplyType & amount);
    void        buyCard(const PlayerID player);
    void        sellCard(const PlayerID player);
    bool        hasSupplyRemaining(const PlayerID player) const;
};

}

