#include "GUIState_Menu.h"
#include "GUIState_Play.h"

#include "Prismata.h"
#include "PrismataAI.h"
#include "Assets.h"
#include "GUIEngine.h"

using namespace Prismata;

GUIState_Menu::GUIState_Menu(GUIEngine & game)
    : GUIState(game)
{
    init("");
}

void GUIState_Menu::init(const std::string & menuConfig)
{
    m_title = "Prismata AI: Select Starting State";

    for (auto & stateName : AIParameters::Instance().getStateNames())
    {
        m_menuStrings.push_back(stateName);
    }

    m_menuText.setFont(Assets::Instance().getFont("Consolas"));
    m_menuText.setCharacterSize(16);
}

void GUIState_Menu::onFrame()
{
    sUserInput();
    sRender();
}

void GUIState_Menu::sUserInput()
{
    sf::Event event;
    while (m_game.window().pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_game.quit();
        }
        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape: 
                { 
                    m_game.quit(); 
                    break; 
                }
                case sf::Keyboard::W: 
                case sf::Keyboard::Up:
                {
                    if (m_selectedMenuIndex > 0) { m_selectedMenuIndex--; }
                    else { m_selectedMenuIndex = m_menuStrings.size() - 1; }
                    break;
                }
                case sf::Keyboard::S: 
                case sf::Keyboard::Down:
                { 
                    m_selectedMenuIndex = (m_selectedMenuIndex + 1) % m_menuStrings.size(); 
                    break; 
                }
                case sf::Keyboard::D: 
                case sf::Keyboard::Return:
                { 
                    auto & stateName = m_menuStrings[m_selectedMenuIndex];
                    m_game.pushState(std::make_shared<GUIState_Play>(m_game, AIParameters::Instance().getState(stateName)));
                    break; 
                }
                default: break;
            }
        }
    }
}

void GUIState_Menu::sRender()
{
    // clear the window to a blue
    m_game.window().setView(m_game.window().getDefaultView());
    m_game.window().clear(sf::Color(0, 0, 0));

    // draw the game title in the top-left of the screen
    m_menuText.setCharacterSize(32);
    m_menuText.setString(m_title);
    m_menuText.setFillColor(sf::Color::White);
    m_menuText.setPosition(sf::Vector2f(12, 5));
    m_game.window().draw(m_menuText);
    
    m_menuText.setCharacterSize(32);
    const int filesPerLine = 38;
    // draw all of the menu options
    for (size_t i = 0; i < m_menuStrings.size(); i++)
    {
        m_menuText.setString(m_menuStrings[i]);
        m_menuText.setFillColor(i == m_selectedMenuIndex ? sf::Color::Yellow : sf::Color(127, 127, 127));
        m_menuText.setPosition(sf::Vector2f(32.0f + (float)(i/filesPerLine)*450, 50.0f + (i%filesPerLine) * (float)m_menuText.getCharacterSize()+2));
        m_game.window().draw(m_menuText);
    }

    // draw the controls in the bottom-left
    m_menuText.setCharacterSize(32);
    m_menuText.setFillColor(sf::Color::Yellow);
    m_menuText.setString("up: w/up   down: s/down   run: d/enter   back: esc");
    m_menuText.setPosition(sf::Vector2f(15, m_game.window().getSize().y - 50));
    m_game.window().draw(m_menuText);

    m_game.window().display();
}