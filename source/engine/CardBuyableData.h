#pragma once

#include "Common.h"
#include "CardType.h"
#include "CardBuyable.h"

namespace Prismata
{

class CardBuyableData 
{
    std::vector<CardBuyable> m_buyableCards;

public:

    CardBuyableData();

    const   CardBuyable &   getCardBuyableByIndex(const CardID cardIndex) const;
            CardBuyable &   getCardBuyableByIndex(const CardID cardIndex);

    const   CardBuyable &   getCardBuyableByID(const CardID cardID) const;
            CardBuyable &   getCardBuyableByID(const CardID cardID);

    const   CardBuyable &   getCardBuyableByType(const CardType type) const;
            CardBuyable &   getCardBuyableByType(const CardType type);
            
    const   CardID          size() const;

    void                    addCardBuyable(const CardBuyable & cardBuyable);
    void                    buyCardByID(const PlayerID player, const CardID cardID);
    void                    buyCardByIndex(const PlayerID player, const CardID cardIndex);
    void                    sellCardByID(const PlayerID player, const CardID cardID);
};

}
