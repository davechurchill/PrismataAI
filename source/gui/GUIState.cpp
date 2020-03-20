#include "GUIState.h"
#include "GUIEngine.h"

using namespace Prismata;

GUIState::GUIState(GUIEngine & game)
    : m_game(game)
{ 
    
}

void GUIState::setPaused(bool paused)
{
    m_paused = paused;
}

const GUIEngine & GUIState::getEngine() const
{
	return m_game;
}

void GUIState::playSound(const std::string& soundName)
{
    Assets::Instance().getSound(soundName).play();
}

size_t GUIState::currentFrame() const
{
    return m_currentFrame;
}

void GUIState::drawLine(double x1, double y1, double x2, double y2, sf::Color color)
{
    sf::Vertex v1(sf::Vector2f((float)x1, (float)y1), color);
    sf::Vertex v2(sf::Vector2f((float)x2, (float)y2), color);
}

void GUIState::drawLine(int x1, int y1, int x2, int y2, sf::Color color)
{
    drawLine((double)x1, (double)y1, (double)x2, (double)y2, color);
}