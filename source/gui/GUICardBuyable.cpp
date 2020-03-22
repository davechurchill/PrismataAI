#include "GUICardBuyable.h"

using namespace Prismata;

const sf::Vector2f BuyableCardSize                = sf::Vector2f(200, 60);
const sf::Vector2f BuyableBorderSize              = sf::Vector2f(0, 0);
const sf::Vector2f BuyableTextureOffset           = sf::Vector2f(BuyableCardSize.x - BuyableCardSize.x / 5, 0);
const sf::Vector2f BuyableCardNameOffset          = sf::Vector2f(0, 3);
const sf::Vector2f BuyableBlackBoxOffset          = sf::Vector2f(BuyableCardSize.x / 5, BuyableCardSize.y - 20);
const sf::Vector2f BuyableADOffset                = BuyableCardSize - sf::Vector2f(5, 5);
const sf::Vector2f BuyableBuyCostOffset           = sf::Vector2f(BuyableCardSize.x - 5, 13);

sf::Color BuyableCardColorUnused(200, 160, 40, 255);
sf::Color BuyableCardColorUsed(40, 100, 220, 225);
sf::Color BuyableBorderColorUnused(120, 100, 25, 255);
sf::Color BuyableBorderColorUsed(25, 50, 120, 200);
sf::Color BuyableTextureColor(255, 255, 255, 200);
sf::Color BuyableBlackBoxColor(0, 0, 0, 170);
sf::Color BuyableCardNameColor(255, 255, 255, 255);
sf::Color BuyableCardNameShadowColor(0, 0, 0, 255);

GUICardBuyable::GUICardBuyable()
{

}
    
GUICardBuyable::GUICardBuyable(const CardBuyable & cb, const sf::Vector2f & p, sf::RenderWindow & window)
    : m_cardBuyable(&cb)
    , m_pos(p)
    , m_layer(0)
    , m_window(&window)
{

}

const bool GUICardBuyable::operator < (const GUICardBuyable & rhs) const
{
    return m_cardBuyable->getType() < rhs.m_cardBuyable->getType();
}

const CardType & GUICardBuyable::getType() const
{
    return m_cardBuyable->getType();
}

const sf::Vector2f & GUICardBuyable::pos() const
{
    return m_pos;
}

void GUICardBuyable::setPosition(const sf::Vector2f & pos)
{
    m_pos = pos;
}

const int GUICardBuyable::getLayer() const
{
    return m_layer;
}

Action GUICardBuyable::onClick(const int & player, const int & phase)
{
    return Action(player, ActionTypes::BUY, m_cardBuyable->getType().getID());
}

const bool GUICardBuyable::isClicked(int x, int y) const
{
    return (x >= pos().x) && (x <= pos().x + BuyableCardSize.x) && (y >= pos().y) && (y <= pos().y + BuyableCardSize.y);
}

void GUICardBuyable::draw(const int layer, const GameState & state)
{
    if (m_pos == sf::Vector2f(-1,-1))
    {
        return;
    }

    sf::Vector2f iconSize(20, 20);
    sf::Vector2f border(4,1);
    sf::Vector2f supplyBuffer(2,0);
    sf::Vector2f supplyOrigin = pos() + BuyableCardNameOffset + sf::Vector2f(0, 40);
    int supplyWidth = 0;
    if (m_cardBuyable->getMaxSupply(0) > 0)
    {
        supplyWidth = ((BuyableCardSize.x - BuyableCardSize.y - 10) / m_cardBuyable->getMaxSupply(0)) - supplyBuffer.x;
    }

    bool canBuy = state.isLegal(Action(state.getActivePlayer(), ActionTypes::BUY, getType().getID()));

    sf::Vector2f ownedPos[2] = {pos() + border + sf::Vector2f(BuyableCardSize.x-30, 15), pos() + border + sf::Vector2f(BuyableCardSize.x-30, 32)};
    CardID numOwned[2] = {state.numCardsOfType(0, m_cardBuyable->getType()), state.numCardsOfType(1, m_cardBuyable->getType()) };
    std::stringstream numOwnedStr[2];
    if (numOwned[0] > 0) numOwnedStr[0] << (int)numOwned[0];
    if (numOwned[1] > 0) numOwnedStr[1] << (int)numOwned[1];

    GUITools::DrawRect(supplyOrigin, supplyOrigin, sf::Color::Green, m_window);
    std::stringstream ss;
    //ss << (int)layer << " " << (int)getType().getID() << " " << (int)_cardBuyable->getIndex() << " " << getType().getUIName();
    ss << getType().getUIName();

    if (canBuy)
    {
        GUITools::DrawRect(pos(), BuyableCardSize - border + sf::Vector2f(1,1), sf::Color::Green, m_window);
    }

    GUITools::DrawRect(pos() + sf::Vector2f(1,0), BuyableCardSize - border, sf::Color(25, 25, 25, 200), m_window);

    // card image
    GUITools::DrawTexturedRect(pos() + sf::Vector2f(BuyableCardSize.x - BuyableCardSize.y, 0), sf::Vector2f(BuyableCardSize.y, BuyableCardSize.y) - border, getType().getUIName(), BuyableTextureColor, m_window);
        
    GUITools::DrawString(pos() + sf::Vector2f(0,-3) + BuyableCardNameOffset + sf::Vector2f(1,1), ss.str(), BuyableCardNameColor, m_window);
        
    // draw buy cost
    GUITools::DrawBuyCost(getType(), pos() + BuyableCardNameOffset + sf::Vector2f(0, 16), iconSize, sf::Vector2f(iconSize.x/2, iconSize.y/2), sf::Vector2f(2, 0), false, m_window);

    // draw remaining supply
    for (size_t s(0); s<m_cardBuyable->getMaxSupply(0); ++s)
    {
        if (supplyWidth > 0)
        {
            bool g = m_cardBuyable->getSupplyRemaining(0) > s;
            bool g2 = m_cardBuyable->getSupplyRemaining(1) > s;

            sf::Vector2f startPos = supplyOrigin + sf::Vector2f(s*(supplyWidth + supplyBuffer.x), 0);
                
            GUITools::DrawRect(startPos, sf::Vector2f(supplyWidth, 3), g ? sf::Color::Green : sf::Color(25, 25, 25, 200), m_window);
            GUITools::DrawRect(startPos + sf::Vector2f(0,3), sf::Vector2f(supplyWidth, 3), g2 ? sf::Color::Red : sf::Color(25, 25, 25, 200), m_window);
        }
    }
    
}
