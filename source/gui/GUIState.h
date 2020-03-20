#pragma once

#include <memory>
#include <string>
#include <SFML/Graphics.hpp>

namespace Prismata
{
class GUIEngine;

class GUIState
{

protected: 
    
    GUIEngine &     m_game;
    bool            m_paused = false;
    size_t          m_currentFrame = 0;

    GUIState(GUIEngine & game);

public:

    virtual void playSound(const std::string& soundName);
    virtual void onFrame() = 0;
    virtual void setPaused(bool paused);
	virtual const GUIEngine & getEngine() const;
    virtual size_t currentFrame() const;

    void drawLine(double x1, double y1, double x2, double y2, sf::Color color);
    void drawLine(int x1, int y1, int x2, int y2, sf::Color color);
};
}