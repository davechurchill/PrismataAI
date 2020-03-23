#include "CardBuyable.h"

using namespace Prismata;

CardBuyable::CardBuyable()
{
   
}

CardBuyable::CardBuyable(const CardType type, const CardID p1MaxSupply, const CardID p2MaxSupply, const CardID p1Spent, const CardID p2Spent)
    : m_type(type)
{
    m_maxSupply[Players::Player_One] = p1MaxSupply;
    m_maxSupply[Players::Player_Two] = p2MaxSupply;
    m_supplyRemaining[Players::Player_One] = p1MaxSupply - p1Spent;
    m_supplyRemaining[Players::Player_Two] = p2MaxSupply - p2Spent;
}

bool CardBuyable::operator < (const CardBuyable & rhs) const
{
    return getType().getName().compare(rhs.getType().getName()) < 0;
}

CardBuyable & CardBuyable::operator = (const CardBuyable & rhs)
{
    if (this != &rhs)
    {
        new (this) CardBuyable(rhs);
    }

    return *this;
}

const CardType CardBuyable::getType() const
{
    return m_type;
}

SupplyType CardBuyable::getSupplyRemaining(const PlayerID player) const
{
    return m_supplyRemaining[player];
}

SupplyType CardBuyable::getMaxSupply(const PlayerID player) const
{
    return m_maxSupply[player];
}

void CardBuyable::setSupplyRemaining(const PlayerID player, const SupplyType & amount)
{
    m_supplyRemaining[player] = amount;
}

void CardBuyable::buyCard(const PlayerID player)
{
    PRISMATA_ASSERT(getSupplyRemaining(player) != 0, "Can't remove from 0 supply");

    m_supplyRemaining[player]--;
}

void CardBuyable::sellCard(const PlayerID player)
{
    PRISMATA_ASSERT(getSupplyRemaining(player) < getMaxSupply(player), "Can't sell a card at max supply");

    m_supplyRemaining[player]++;
}

bool CardBuyable::hasSupplyRemaining(const PlayerID player) const
{
    return getSupplyRemaining(player) > 0;
}