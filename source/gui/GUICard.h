#pragma once

#include "Common.h"

#include "Prismata.h"
#include <SFML/Graphics.hpp>

namespace Prismata
{
 
class GUICard
{
    sf::RenderWindow *  m_window = nullptr;
    const Card *        m_card   = nullptr;
    sf::Vector2f        m_pos    = {-1, -1};
    int                 m_layer  = -1;

public:

    GUICard();
    GUICard(const Card & card, const sf::Vector2f & p, sf::RenderWindow & window);

    const bool operator < (const GUICard & rhs) const;

    const Card *        getCard();
    void                draw(const int layer, const GameState & state, bool isFirstInStack);
    void                setPosition(const sf::Vector2f & pos);
    const int           getLayer() const;
    sf::Vector2f pos() const;
    const bool          isClicked(int x, int y) const;
    Action              onClick(const GameState & currentState);

    const int           getLane() const;

    static void SetCardSize(sf::Vector2f size);
    static sf::Vector2f GetCardSize();
};

class GUICardNewSort
{
public:
    bool operator() (GUICard & a, GUICard & b);
};
}
