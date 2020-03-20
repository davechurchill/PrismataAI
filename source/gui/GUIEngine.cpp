#include "GUIEngine.h"
#include "Assets.h"

#include "Prismata.h"
#include "GUIState_Menu.h"
#include "GUIState_Menu.h"

#include <fstream>
#include <iostream>

using namespace Prismata;

GUIEngine::GUIEngine()
{
    init();
}

void GUIEngine::init()
{
    // load card image textures
    const std::vector<CardType> & types = CardTypes::GetAllCardTypes();
    for (size_t i(0); i<types.size(); ++i)
    {
        Assets::Instance().addTexture(types[i].getUIName(), "asset/images/cards/" + types[i].getImageFileName());
    }

    for (int n(0); n<=20; ++n)
    {
        std::stringstream ss;
        ss << "asset/images/icons/numbers/" << n << ".png";
        Assets::Instance().addTexture(std::to_string(n), ss.str());
    }

    // load assets
    loadAssets("asset/config/assets.txt");
    
    m_window.create(sf::VideoMode(1600, 900), "Prismata AI");
    m_window.setFramerateLimit(60);

    pushState(std::make_shared<GUIState_Menu>(*this));
}

void GUIEngine::loadAssets(const std::string & assetFilePath)
{
    std::ifstream fin(assetFilePath);
    std::string token, name, file;

    while (fin.good())
    {
        fin >> token;

        if (token == "Texture")
        {
            fin >> name >> file;
            Assets::Instance().addTexture(name, file);
        }
        else if (token == "Font")
        {
            fin >> name >> file;
            Assets::Instance().addFont(name, file);
        }
        else
        {
            std::cerr << "ERROR: Unknown asset token: " << token << "\n";
            std::exit(-1);
        }
    }
}

bool GUIEngine::isRunning()
{ 
    return m_running && m_window.isOpen();
}

sf::RenderWindow & GUIEngine::window()
{
    return m_window;
}

void GUIEngine::run()
{
    while (isRunning())
    {
        onFrame();

        if (m_states.empty())
        {
            quit();
        }
    }
}

void GUIEngine::pushState(std::shared_ptr<GUIState> state)
{
    m_statesToPush.push_back(state);
}

void GUIEngine::popState()
{
    m_popStates++;
}

void GUIEngine::onFrame()
{
    if (!isRunning()) { return; }
    m_updates++;
    
    // pop however many states off the state stack as we have requested
    for (size_t i = 0; i < m_popStates; i++)
    {
        if (!m_states.empty())
        {
            m_states.pop_back();
        }
    }
    // reset the state stack pop counter
    m_popStates = 0;

    // push any requested states onto the stack
    for (size_t i = 0; i < m_statesToPush.size(); i++)
    {
        m_states.push_back(m_statesToPush[i]);
    }
    m_statesToPush.clear();

    // call update on the top of the stack (current state)
    if (!m_states.empty())
    {
        m_states.back()->onFrame();
    }
}

void GUIEngine::quit()
{
    m_running = false;
}
