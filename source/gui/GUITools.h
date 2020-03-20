#pragma once

#include "Common.h"

#include "Resources.h"
#include "Card.h"
#include <SFML/Graphics.hpp>

namespace Prismata
{
    namespace GUITools
    {
        const int FLIP_VERTICAL = 1;
        const int FLIP_HORIZONTAL = 2;

        std::string GetClipboardText();

        // NEW: DONT TOUCH
        void DrawRect(sf::Vector2f tl, sf::Vector2f size, sf::Color color, sf::RenderWindow * window);
        void DrawTexturedRect(sf::Vector2f tl, sf::Vector2f size, const std::string & tex, sf::Color color, sf::RenderWindow * window);
        void DrawString(sf::Vector2f p, const std::string & str, sf::Color color, sf::RenderWindow * window, int size = 12);
        void DrawIconAndText(const sf::Vector2f & tl, const sf::Vector2f & size, const std::string & tex1, const std::string & tex2, sf::Color color, sf::RenderWindow * window);
        void DrawMana(const Resources & resource, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window);
        void DrawBuyCost(const CardType & cardType, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window);
        void DrawLine(const sf::Vector2f & p1, const sf::Vector2f & p2, sf::Color color, sf::RenderWindow * window);
        void DrawMouseOverPane(const CardType & type, const sf::Vector2f & pos, const Card * card, sf::RenderWindow * window);
        void DrawScriptEffect(const Script & script, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window);
        void DrawScriptCost(const Script & script, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window);
    }
}
