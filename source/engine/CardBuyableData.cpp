#include "CardData.h"

using namespace Prismata;

CardBuyableData::CardBuyableData()
{
    
}

const CardBuyable & CardBuyableData::getCardBuyableByIndex(const CardID cardIndex) const
{
    PRISMATA_ASSERT(cardIndex < m_buyableCards.size(), "cardIndex exceeds capacity, cardIndex=%d, capacity=%d", cardIndex, m_buyableCards.size());

    return m_buyableCards[cardIndex];
}

CardBuyable & CardBuyableData::getCardBuyableByIndex(const CardID cardIndex)
{
    PRISMATA_ASSERT(cardIndex < m_buyableCards.size(), "cardIndex exceeds capacity, cardIndex=%d, capacity=%d", cardIndex, m_buyableCards.size());

    return m_buyableCards[cardIndex];
}

const CardBuyable & CardBuyableData::getCardBuyableByID(const CardID cardID) const
{
    for (size_t i(0); i < m_buyableCards.size(); ++i)
    {
        if (m_buyableCards[i].getType() == cardID)
        {
            return m_buyableCards[i];
        }
    }

    PRISMATA_ASSERT(false, "Couldn't find card by ID");
    return m_buyableCards[0];
}

CardBuyable & CardBuyableData::getCardBuyableByID(const CardID cardID)
{
    for (size_t i(0); i < m_buyableCards.size(); ++i)
    {
        if (m_buyableCards[i].getType() == cardID)
        {
            return m_buyableCards[i];
        }
    }

    PRISMATA_ASSERT(false, "Couldn't find card by ID");
    return m_buyableCards[0];
}

const CardBuyable & CardBuyableData::getCardBuyableByType(const CardType type) const
{
    return getCardBuyableByID(type.getID());
}

CardBuyable & CardBuyableData::getCardBuyableByType(const CardType type)
{
    return getCardBuyableByID(type.getID());
}

const CardID CardBuyableData::size() const
{
    return m_buyableCards.size();
}

void CardBuyableData::buyCardByID(const PlayerID player, const CardID cardID)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    getCardBuyableByID(cardID).buyCard(player);
}

void CardBuyableData::sellCardByID(const PlayerID player, const CardID cardID)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    getCardBuyableByID(cardID).sellCard(player);
}

void CardBuyableData::buyCardByIndex(const PlayerID player, const CardID cardIndex)
{
    PRISMATA_ASSERT(player < 2, "player exceeds num players, player=%d, numplayers=%d", player, 2);

    getCardBuyableByIndex(cardIndex).buyCard(player);
}

void CardBuyableData::addCardBuyable(const CardBuyable & cardBuyable)
{
    m_buyableCards.push_back(cardBuyable);
}
