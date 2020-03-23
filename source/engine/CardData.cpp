#include "CardData.h"

using namespace Prismata;

CardData::CardData()
{
    m_cardTypeCounts[0] = CardIDVector(CardTypes::GetAllCardTypes().size(), 0);
    m_cardTypeCounts[1] = CardIDVector(CardTypes::GetAllCardTypes().size(), 0);
}

const Card & CardData::getCard(const PlayerID player, const CardID cardIndex) const
{ 
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);
    PRISMATA_ASSERT(cardIndex < numCards(player), "cardIndex exceeds live card capacity: cardIndex=%d, capacity=%d", cardIndex, numCards(player));
    PRISMATA_ASSERT(cardIndex < m_allCards.size(), "cardIndex exceeds all card capacity: cardIndex=%d, capacity=%d", cardIndex, m_allCards.size());
    PRISMATA_ASSERT(!m_allCards[m_liveCardIDs[player][cardIndex]].isDead(), "Trying to return a dead card: (%d, %d) %s", (int)player, (int)cardIndex, m_allCards[m_liveCardIDs[player][cardIndex]].getType().getName().c_str());

    return m_allCards[m_liveCardIDs[player][cardIndex]];
}

Card & CardData::getCard(const PlayerID player, const CardID cardIndex)
{
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);
    PRISMATA_ASSERT(cardIndex < numCards(player), "cardIndex exceeds live card capacity: cardIndex=%d, capacity=%d", cardIndex, numCards(player));
    PRISMATA_ASSERT(cardIndex < m_allCards.size(), "cardIndex exceeds all card capacity: cardIndex=%d, capacity=%d", cardIndex, m_allCards.size());
    PRISMATA_ASSERT(!m_allCards[m_liveCardIDs[player][cardIndex]].isDead(), "Trying to return a dead card: (%d, %d) %s", (int)player, (int)cardIndex, m_allCards[m_liveCardIDs[player][cardIndex]].getType().getName().c_str());

    return m_allCards[m_liveCardIDs[player][cardIndex]];
}
    
const Card & CardData::getKilledCard(const PlayerID player, const CardID cardIndex) const
{ 
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);
    PRISMATA_ASSERT(cardIndex < numKilledCards(player), "cardIndex exceeds live card capacity: cardIndex=%d, capacity=%d", cardIndex, numCards(player));
    PRISMATA_ASSERT(cardIndex < m_allCards.size(), "cardIndex exceeds all card capacity: cardIndex=%d, capacity=%d", cardIndex, m_allCards.size());
    PRISMATA_ASSERT(m_allCards[m_killedCardIDs[player][cardIndex]].isDead(), "Trying to return a live card: (%d, %d) %s", (int)player, (int)cardIndex, m_allCards[m_killedCardIDs[player][cardIndex]].getType().getName().c_str());

    return m_allCards[m_killedCardIDs[player][cardIndex]];
}

Card & CardData::getKilledCard(const PlayerID player, const CardID cardIndex)
{ 
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);
    PRISMATA_ASSERT(cardIndex < numKilledCards(player), "cardIndex exceeds live card capacity: cardIndex=%d, capacity=%d", cardIndex, numCards(player));
    PRISMATA_ASSERT(cardIndex < m_allCards.size(), "cardIndex exceeds all card capacity: cardIndex=%d, capacity=%d", cardIndex, m_allCards.size());
    PRISMATA_ASSERT(m_allCards[m_killedCardIDs[player][cardIndex]].isDead(), "Trying to return a live card: (%d, %d) %s", (int)player, (int)cardIndex, m_allCards[m_killedCardIDs[player][cardIndex]].getType().getName().c_str());

    return m_allCards[m_killedCardIDs[player][cardIndex]];
}

const CardID CardData::numCards(const PlayerID player) const
{
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);

    return (CardID)m_liveCardIDs[player].size();
}

const CardID CardData::numKilledCards(const PlayerID player) const
{
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);

    return (CardID)m_killedCardIDs[player].size();
}

const Card & CardData::getCardByID(const CardID id) const
{
    PRISMATA_ASSERT(id < m_allCards.size(), "id exceeds capacity: id=%d, capacity=%d", id, m_allCards.size());

    return m_allCards[id];
}

Card & CardData::getCardByID(const CardID id)
{
    PRISMATA_ASSERT(id < m_allCards.size(), "id exceeds capacity: id=%d, capacity=%d", id, m_allCards.size());

    return m_allCards[id];
}

Card & CardData::addCard(const Card & card)
{
    const CardID nextCardID = getFreeCardID();
    m_allCards[nextCardID] = card;
    m_allCards[nextCardID].setID(nextCardID);

    Card & cardInVector = m_allCards[nextCardID];

    //_allCards.push_back(card);
    //_allCards.back().setID(_allCards.size()-1);
    //Card & cardInVector = _allCards.back();

    m_liveCardIDs[cardInVector.getPlayer()].push_back(cardInVector.getID());
    m_cardTypeCounts[card.getPlayer()][card.getType().getID()]++;
    
    return cardInVector;
}

void CardData::removeLiveCard(const PlayerID player, const CardID cardIndex)
{
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);    
    PRISMATA_ASSERT(cardIndex < numCards(player), "cardIndex=%d, size=%d", cardIndex, numCards(player));

    const CardType type = getCardByID(m_liveCardIDs[player][cardIndex]).getType();
    PRISMATA_ASSERT(getCardTypeCount(player, type) > 0, "removing a card with 0 type count");

    m_cardTypeCounts[player][type.getID()]--;
    m_liveCardIDs[player].erase(m_liveCardIDs[player].begin() + cardIndex);
}

void CardData::removeKilledCard(const PlayerID player, const CardID cardIndex)
{
    PRISMATA_ASSERT(player < 2, "player exceeds capacity: player=%d, total players=%d", player, 2);    
    PRISMATA_ASSERT(cardIndex < m_killedCardIDs[player].size(), "cardIndex=%d, size=%d", cardIndex, numCards(player));

    m_killedCardIDs[player].erase(m_killedCardIDs[player].begin() + cardIndex);
}

void CardData::removeLiveCardByID(const CardID cardID)
{
    const PlayerID player = getCardByID(cardID).getPlayer();
    const CardID cards = numCards(player);
    
    for (CardID c(0); c < cards; ++c)
    {
        if (m_liveCardIDs[player][c] == cardID)
        {
            removeLiveCard(player, c);
            return;
        }
    }

    PRISMATA_ASSERT(false, "Tried to remove a Card that didn't exist: %d", cardID);
}

void CardData::removeKilledCardByID(const CardID cardID)
{
    const PlayerID player = getCardByID(cardID).getPlayer();
    const CardID cards = numKilledCards(player);

    for (CardID c(0); c < cards; ++c)
    {
        if (m_killedCardIDs[player][c] == cardID)
        {
            removeKilledCard(player, c);
            return;
        }
    }

    PRISMATA_ASSERT(false, "Tried to remove a Card that didn't exist: %d %s", cardID, getCardByID(cardID).getType().getUIName().c_str());
}

void CardData::killCardByID(const CardID cardID, const int causeOfDeath)
{
    Card & card = getCardByID(cardID);

    if (!card.isDead())
    {
        card.kill(causeOfDeath);
    }

    removeLiveCardByID(cardID);

    if (causeOfDeath != CauseOfDeath::Deleted)
    {
        m_killedCardIDs[card.getPlayer()].push_back(cardID);
    }
}

void CardData::undoKill(const CardID cardID)
{
    Card & card = getCardByID(cardID);
    card.undoKill();

    const PlayerID player = card.getPlayer();

    PRISMATA_ASSERT(m_killedCardIDs[player].size() > 0, "Player has no killed cards to undo killing %d %s", (int)cardID, getCardByID(cardID).getType().getName().c_str());

    // remove it from the killed index
    removeKilledCardByID(cardID);

    // and add it to the live index
    m_liveCardIDs[player].push_back(cardID);
    m_cardTypeCounts[player][getCardByID(cardID).getType().getID()]++;
}

void CardData::sellCardByID(const CardID cardID)
{
    Card & card = getCardByID(cardID);

    // remove the card from whatever list it is in
    if (card.isDead())
    {
        removeKilledCardByID(cardID);
    }
    else
    {
        removeLiveCardByID(cardID);
    }

    // increase the supply
    m_cardsBuyable.sellCardByID(card.getPlayer(), card.getType().getID());

    // remove the card and all cards it created from play
    const auto & createdIDs = card.getCreatedCardIDs();
    for (size_t i(0); i < createdIDs.size(); ++i)
    {
        getCardByID(createdIDs[i]).setInPlay(false);
    }

    card.setInPlay(false);
    
}

Card & CardData::buyCardByID(const PlayerID player, const CardID cardID)
{
    PRISMATA_ASSERT(getCardBuyableByID(cardID).getSupplyRemaining(player) > 0, "Trying to buy a card with no supply remaining");

    m_cardsBuyable.buyCardByID(player, cardID);

    return addCard(Card(m_cardsBuyable.getCardBuyableByID(cardID).getType(), player, CardCreationMethod::Bought, 0, 0));
}

const CardBuyable & CardData::getCardBuyableByIndex(const CardID cardIndex) const
{
    return m_cardsBuyable.getCardBuyableByIndex(cardIndex);
}

CardBuyable & CardData::getCardBuyableByIndex(const CardID cardIndex)
{
    return m_cardsBuyable.getCardBuyableByIndex(cardIndex);
}

const CardBuyable & CardData::getCardBuyableByID(const CardID cardID) const
{
    return m_cardsBuyable.getCardBuyableByID(cardID);
}

CardBuyable & CardData::getCardBuyableByID(const CardID cardID)
{
    return m_cardsBuyable.getCardBuyableByID(cardID);
}

const CardBuyable & CardData::getCardBuyableByType(const CardType type) const
{
    return m_cardsBuyable.getCardBuyableByType(type);
}

CardBuyable & CardData::getCardBuyableByType(const CardType type)
{
    return m_cardsBuyable.getCardBuyableByType(type);
}

const CardID CardData::numCardsBuyable() const
{
    return m_cardsBuyable.size();
}

void CardData::addBuyableCardType(const CardType type)
{
    m_cardsBuyable.addCardBuyable(CardBuyable(type, type.getSupply(), type.getSupply(), 0, 0));
}

void CardData::addBuyableCard(const CardBuyable & cardBuyable)
{
    m_cardsBuyable.addCardBuyable(cardBuyable);
}

const size_t CardData::getMemoryUsed() const
{
    size_t units = m_allCards.size() * sizeof(Card);
    size_t ids = numCards(0)*sizeof(CardID) + numCards(1)*sizeof(CardID);

    return units + ids;
}

void CardData::removeKilledCards()
{
    for (PlayerID p(0); p < 2; ++p)
    {
        for (size_t i(0); i < numKilledCards(p); ++i)
        {
            getKilledCard(p, i).setInPlay(false);
        }

        m_killedCardIDs[p].clear();
    }
}

void CardData::validateUnitArrays()
{
    for (PlayerID p(0); p < 2; ++p)
    {
        for (size_t c(0); c < numCards(p); ++c)
        {
            PRISMATA_ASSERT(!getCard(p, c).isDead(), "Card in live array is dead");
        }

        for (size_t c(0); c < numKilledCards(p); ++c)
        {
            PRISMATA_ASSERT(getKilledCard(p, c).isDead(), "Card in dead array is alive");
        }
    }
}

const CardID CardData::getFreeCardID()
{
    for (CardID i(0); i < m_allCards.size(); ++i)
    {
        if (!m_allCards[i].isInPlay())
        {
            return i;
        }
    }

    m_allCards.push_back(Card());
    return m_allCards.size()-1;
}

const CardID CardData::getCardTypeCount(const PlayerID player, const CardType type) const
{
    return m_cardTypeCounts[player][type.getID()];
}

const CardIDVector & CardData::getCardIDs(const PlayerID player) const
{
    return m_liveCardIDs[player];
}

const CardIDVector & CardData::getKilledCardIDs(const PlayerID player) const
{
    return m_killedCardIDs[player];
}