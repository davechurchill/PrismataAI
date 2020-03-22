#pragma once

#include "GUIState.h"
#include "Assets.h"

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

namespace Prismata
{
class GUIEngine
{

protected:

    std::vector<std::shared_ptr<GUIState>>  m_states;
    std::vector<std::shared_ptr<GUIState>>  m_statesToPush;
    sf::RenderWindow                        m_window;
    size_t                                  m_popStates = 0;
    bool                                    m_running = true;
    size_t                                  m_updates = 0;
	size_t									m_currentFrame = 0;

    void onFrame();

public:
    
    GUIEngine();

    void init();
    void loadAssets(const std::string & path);

    void pushState(std::shared_ptr<GUIState> state);
    void popState();

    void quit();
    void run();

    sf::RenderWindow & window();
    bool isRunning();
};
}