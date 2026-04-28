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
    return m_buyableCards[getCardBuyableIndexByID(cardID)];
}

CardBuyable & CardBuyableData::getCardBuyableByID(const CardID cardID)
{
    return m_buyableCards[getCardBuyableIndexByID(cardID)];
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

CardID CardBuyableData::getCardBuyableIndexByID(const CardID cardID) const
{
    if (hasCardBuyableByID(cardID))
    {
        return m_cardIDToBuyableIndex[cardID];
    }

    PRISMATA_ASSERT(false, "Couldn't find card by ID");
    return 0;
}

bool CardBuyableData::hasCardBuyableByID(const CardID cardID) const
{
    return cardID < m_cardIDToBuyableIndex.size() && m_cardIDToBuyableIndex[cardID] != InvalidBuyableIndex;
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
    const CardID cardTypeID = cardBuyable.getType().getID();

    if (cardTypeID >= m_cardIDToBuyableIndex.size())
    {
        m_cardIDToBuyableIndex.resize(cardTypeID + 1, InvalidBuyableIndex);
    }

    const bool alreadyBuyable = hasCardBuyableByID(cardTypeID);
    PRISMATA_ASSERT(!alreadyBuyable, "Card already exists in buyable list");

    if (!alreadyBuyable)
    {
        m_cardIDToBuyableIndex[cardTypeID] = (CardID)m_buyableCards.size();
    }

    m_buyableCards.push_back(cardBuyable);
}
