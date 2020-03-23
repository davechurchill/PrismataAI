#pragma once

#include "Common.h"
#include "Card.h"
#include "CardBuyableData.h"

namespace Prismata
{

typedef std::vector<CardID> CardIDVector;
typedef std::vector<Card>   CardVector;

class CardData
{
    CardVector      m_allCards;          // stores all Card instances, indexed by Card.getID()
    CardIDVector    m_liveCardIDs[2];    // stores the indices of live cards for each player
    CardIDVector    m_killedCardIDs[2];  // stores the indices of live cards for each player
    CardIDVector    m_cardTypeCounts[2]; // counts of each type
    CardBuyableData m_cardsBuyable;      // stores all data about buyable cards    

    void removeLiveCard(const PlayerID player, const CardID cardIndex);
    void removeKilledCard(const PlayerID player, const CardID cardIndex);

    const CardID getFreeCardID();

public:

    CardData();


    const   Card &          getCard(const PlayerID player, const CardID cardIndex) const;
            Card &          getCard(const PlayerID player, const CardID cardIndex);

    const   Card &          getCardByID(const CardID id) const;
            Card &          getCardByID(const CardID id);

    const   Card &          getKilledCard(const PlayerID player, const CardID cardIndex) const;
            Card &          getKilledCard(const PlayerID player, const CardID cardIndex);

    const   CardID          numCards(const PlayerID player) const;
    const   CardID          numKilledCards(const PlayerID player) const;
    const   CardID          getCardTypeCount(const PlayerID player, const CardType type) const;

    const   CardIDVector &  getCardIDs(const PlayerID player) const;
    const   CardIDVector &  getKilledCardIDs(const PlayerID player) const;
    const   CardID          numCardsBuyable() const;
    const   CardBuyable &   getCardBuyableByIndex(const CardID cardIndex) const;
            CardBuyable &   getCardBuyableByIndex(const CardID cardIndex);
    const   CardBuyable &   getCardBuyableByID(const CardID cardID) const;
            CardBuyable &   getCardBuyableByID(const CardID cardID);
    const   CardBuyable &   getCardBuyableByType(const CardType type) const;
            CardBuyable &   getCardBuyableByType(const CardType type);

            Card &          addCard(const Card & card);
            Card &          buyCardByID(const PlayerID player, const CardID cardBuyableIndex);
    void                    sellCardByID(const CardID cardID);
    void                    addBuyableCardType(const CardType type);
    void                    addBuyableCard(const CardBuyable & type);
    void                    killCardByID(const CardID cardID, const int causeOfDeath);
    void                    removeKilledCards();
    void                    undoKill(const CardID cardID);
    void                    removeLiveCardByID(const CardID cardID);
    void                    removeKilledCardByID(const CardID cardID);
    void                    validateUnitArrays();
    const   size_t          getMemoryUsed() const;
};

}

