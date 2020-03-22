#pragma once

#include "Common.h"

#include "Card.h"
#include "GUITools.h"
#include "CardBuyable.h"
#include "Action.h"
#include "GameState.h"

namespace Prismata
{
 
class GUICardBuyable
{
    sf::RenderWindow *  m_window        = nullptr;
    const CardBuyable * m_cardBuyable   = nullptr;
    sf::Vector2f        m_pos           = {-1, -1};
    int                 m_layer         = 0;
    bool                m_selected      = false;
    
public:

    GUICardBuyable();
    GUICardBuyable(const CardBuyable & type, const sf::Vector2f & p, sf::RenderWindow & window);

    const bool operator < (const GUICardBuyable & rhs) const;

    const CardType &     getType() const;
    const int            getLayer() const;
    const sf::Vector2f & pos() const;
    const bool           isClicked(int x, int y) const;

    void    draw(const int layer, const GameState & state);
    void    setPosition(const sf::Vector2f & pos);
    Action  onClick(const int & player, const EnumType & phase);
};

class GUICardBuyableNewSort
{
public:
    bool operator() (GUICardBuyable & a, GUICardBuyable & b)
    {
        return a.getType() < b.getType();
    }
};
}
