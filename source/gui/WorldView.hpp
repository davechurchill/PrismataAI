#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>

class WorldView
{
    Vec2 m_pos;
    Vec2 m_size;
    Vec2 m_center;
    Vec2 m_scroll;
    Vec2 m_window;

    double m_scrollDecel = 0.92;
    double m_stopScrollSpeed = 2;

    Vec2 m_savedPos;
    Vec2 m_savedSize;

public:

    WorldView()
    {
        
    }

    void setWindowSize(Vec2 windowSize)
    {
        m_window = windowSize;
    }

    void update()
    {
        move(m_scroll);
        m_scroll *= (m_scroll.length() >= m_stopScrollSpeed) ? m_scrollDecel : 0;
    }

    void scroll(Vec2 scroll)
    {
        m_scroll = scroll;
    }
    
    void move(Vec2 move)
    {
        m_pos += move;
        m_center += move;
    }

    Vec2 windowToWorld(Vec2 screen)
    {
        Vec2 ratio = { screen.x / m_window.x, screen.y / m_window.y };
        return { m_pos.x + ratio.x * m_size.x, m_pos.y + ratio.y * m_size.y };
    }

    void moveTo(Vec2 moveTo)
    {
        m_pos = moveTo;
        m_center = m_pos + m_size / 2;
    }

    void saveView()
    {
        m_savedPos = m_pos;
        m_savedSize = m_size;
    }

    void loadView()
    {
        m_pos = m_savedPos;
        m_size = m_savedSize;
        m_center = m_pos + m_size / 2;
        stopScroll();
    }
    
    // zoom by a given amount, maintain center
    void zoom(double factor)
    {
        m_size *= factor;
        m_pos   = m_center - m_size / 2;
    }

    void zoomTo(double factor, Vec2 target)
    {
        Vec2 pRatio = target / m_window;
        Vec2 oldPos = m_pos + m_size * pRatio;

        m_size *= factor;
        m_pos = m_center - m_size / 2;

        Vec2 newPos = m_pos + m_size * pRatio;
        move(oldPos -newPos);
    }
    
    void setView(const sf::View& view)
    {
        m_center = { view.getCenter().x, view.getCenter().y };
        m_size   = { view.getSize().x, view.getSize().y };
        m_pos    = m_center - m_size / 2;
    }

    void stopScroll()
    {
        m_scroll = Vec2();
    }

    sf::View getSFMLView()
    {
        return sf::View(sf::FloatRect((float)m_pos.x, (float)m_pos.y, (float)m_size.x, (float)m_size.y));
    }

    const Vec2& pos() const { return m_pos; }
    const Vec2& center() const { return m_center; }
    const Vec2& size() const { return m_size; }
    const Vec2& savedPos() const { return m_savedPos; }
    const Vec2& savedSize() const { return m_savedSize; }
};