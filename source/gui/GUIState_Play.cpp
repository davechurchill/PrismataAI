#include "GUIState_Play.h"
#include "GUIEngine.h"
#include "WorldView.hpp"
#include "GUITools.h"
#include "AITools.h"
#include "PrismataAI.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <thread>

using namespace Prismata;

GUIState_Play::GUIState_Play(GUIEngine & game, const GameState & state)
    : GUIState(game)
    , m_currentState(state)
{
    m_stateHistory.push_back(state);

    m_view.setWindowSize(Vec2(m_game.window().getSize().x, m_game.window().getSize().y));
    m_view.setView(m_game.window().getView());
        
    m_text.setFont(Assets::Instance().getFont("Consolas"));
    m_text.setPosition(10, 5);
    m_text.setCharacterSize(10);

    loadPlayers();
}

void GUIState_Play::init()
{

}

void GUIState_Play::setState(const GameState & state)
{
    m_currentState = state;
    setGUICards();
    setCardPositions();
}

void GUIState_Play::loadPlayers()
{
    const std::vector<std::string> & playerNames = AIParameters::Instance().getPlayerNames();
    for (size_t i(0); i < playerNames.size(); ++i)
    {
        for (PlayerID p(0); p < 2; ++p)
        {
            m_players[p][playerNames[i]] = AIParameters::Instance().getPlayer(p, playerNames[i]);
        }
    }
}

void GUIState_Play::onFrame()
{
    m_view.update();
    setState(m_currentState);
    sRender(); 
    sUserInput();
    m_currentFrame++;
}

void GUIState_Play::setGUICards()
{
    m_guiCards.clear();
    m_guiCardsBuyable.clear();

    for (PlayerID player=0; player<2; ++player)
    {
        for (const auto & cardID : m_currentState.getCardIDs(player))
        {
            const Card & card = const_cast<const GameState &>(m_currentState).getCardByID(cardID);
            m_guiCards.push_back(GUICard(card, {-1, -1}, m_game.window()));
        }

        for (const auto & cardID : m_currentState.getKilledCardIDs(player))
        {
            const Card & card = const_cast<const GameState &>(m_currentState).getCardByID(cardID);

            if (card.getAliveStatus() == AliveStatus::KilledThisTurn)
            {
                m_guiCards.push_back(GUICard(card, {-1, -1}, m_game.window()));
            }
        }
    }

    for (CardID cb(0); cb < m_currentState.numCardsBuyable(); ++cb)
    {
        const CardBuyable & cardBuyable = const_cast<const GameState &>(m_currentState).getCardBuyableByIndex(cb);
        m_guiCardsBuyable.push_back(GUICardBuyable(cardBuyable, sf::Vector2f(-1,-1), m_game.window()));
    }

    std::sort(m_guiCards.begin(), m_guiCards.end());
}

void GUIState_Play::setCardPositions()
{
    auto CardSize = GUICard::GetCardSize();sf::Vector2f StatusIconSize(CardSize.x / 5, CardSize.y / 5);
    const sf::Vector2f BuyablePaneSize(200, 0);
    const sf::Vector2f BuyableCardSize(200, 60);

    sf::Vector2f buffer = StatusIconSize;
    sf::Vector2f sameBuffer(- 4*CardSize.x/5 , 10);
    sf::Vector2f droneBuffer(- 4.7*CardSize.x/5 , 10);
    sf::Vector2f origin[3][2];

    double droneDiff = sameBuffer.x - droneBuffer.x;
    double droneDiffDelta = droneDiff / 15.0;

    float midX = BuyablePaneSize.x + ((m_game.window().getSize().x - BuyablePaneSize.x) / 2);
    sf::Vector2f mid[2] = { sf::Vector2f(midX, m_game.window().getSize().y/4), sf::Vector2f(sf::Vector2f(midX, 3*m_game.window().getSize().y/4)) };

    float bottomBufferHeight = 60; // how many pixels card bottom will be from bottom of screen
    float playerAreaHeight = m_game.window().getSize().y/2;
    float laneVerticalBuffer = (playerAreaHeight - bottomBufferHeight - 3*CardSize.y) / 4;

    origin[0][0] = sf::Vector2f(0, mid[0].y + CardSize.y );

    for (int i=0; i < 3; ++i)
    {
        origin[i][1] = BuyablePaneSize + sf::Vector2f(0, (m_game.window().getSize().y/2) - (i+1)*laneVerticalBuffer - (i+1)*CardSize.y);
        origin[i][0] = BuyablePaneSize + sf::Vector2f(0, (m_game.window().getSize().y/2) + (i+1)*laneVerticalBuffer + i*CardSize.y);
    }

    CardType lastType;
    
    for (PlayerID player=0; player < 2; ++player)
    {
        sf::Vector2f currentPos[3] = {origin[0][player], origin[1][player], origin[2][player]};
        bool first[3] = {true, true, true};

        for (CardID c(0); c<m_guiCards.size(); ++c)
        {
            int lane = m_guiCards[c].getLane();
            
            if (m_guiCards[c].getCard()->getPlayer() == player)
            {
                bool sameType = (m_guiCards[c].getCard()->getType() == lastType);
                bool droneSame = sameType && lastType.getUIName() == "Drone";
                GUICard * _mouseOverCard = nullptr;
                bool mouseOverType = _mouseOverCard ? (_mouseOverCard->getCard()->getType() == lastType) : false;
                sf::Vector2f buf = sameType ? sameBuffer : buffer;
                if (!first[lane]) { currentPos[lane]  = currentPos[lane] + sf::Vector2f(CardSize.x + buf.x, 0); } 
                m_guiCards[c].setPosition(currentPos[lane]);
                lastType = m_guiCards[c].getCard()->getType();
                first[lane] = false;
            }
        }

        lastType = CardType();

        // fix the lanes to be centered
        float laneMids[3];
        for (int i=0; i < 3; ++i)
        {
            laneMids[i] = origin[i][player].x + (currentPos[i].x - origin[i][player].x + CardSize.x)/2;
        }

        for (CardID c(0); c < m_guiCards.size(); ++c)
        {
            if (m_guiCards[c].getCard()->getPlayer() == player)
            {
                m_guiCards[c].setPosition(m_guiCards[c].pos() + sf::Vector2f(mid[1].x - laneMids[m_guiCards[c].getLane()], 0));
            }
        }
    }

    sf::Vector2f buyableOrigin = sf::Vector2f(0,0);

    sf::Vector2f currentPos = buyableOrigin;
    
    for (CardID c(0); c<m_guiCardsBuyable.size(); ++c)
    {
        if (!m_drawBaseSetCards && CardTypes::IsBaseSet(m_guiCardsBuyable[c].getType())) { continue; }
        if (m_drawBaseSetCards && !CardTypes::IsBaseSet(m_guiCardsBuyable[c].getType())) { continue; }

        m_guiCardsBuyable[c].setPosition(currentPos);

        currentPos = currentPos + sf::Vector2f(0, BuyableCardSize.y);
    }
}

void GUIState_Play::doGUIMove(const Move & move, int delayMS)
{
    m_doingAIMove = true;

    for (size_t i(0); i < move.size(); ++i)
    {
        const Action & action = move.getAction(i);

        doGUIAction(action, delayMS);

        if (delayMS > 0) { onFrame(); }
    }

    m_doingAIMove = false;
}

void GUIState_Play::doGUIAction(const Action & action, int delayMS)
{
    if (!m_currentState.isLegal(action)) { return; }
    m_currentState.doAction(action);
    setState(m_currentState);
    m_stateHistory.push_back(m_currentState);
    if (delayMS > 0) { std::this_thread::sleep_for(std::chrono::milliseconds(delayMS)); }
}

void GUIState_Play::rewindToPreviousState()
{
    if (m_stateHistory.size() == 1) 
    { 
        std::cout << "Cannot rewind from starting state\n";
        return; 
    }

    m_stateHistory.pop_back();
    setState(m_stateHistory.back());
}

void GUIState_Play::endCurrentPhase()
{
    const Action space(m_currentState.getActivePlayer(), ActionTypes::END_PHASE, 0);
    if (m_currentState.isLegal(space))
    doGUIAction(space); 
}

void GUIState_Play::activateWorkers()
{
    for (CardID gc(0); gc < m_guiCards.size(); ++gc)
    {
        GUICard & guiCard = m_guiCards[gc];
        const Card * card = guiCard.getCard();
        bool isDrone = (card->getType().getUIName() == "Drone" || card->getType().getUIName() == "Doomed Drone");
        if (card->getPlayer() == m_currentState.getActivePlayer() && isDrone && (!card->getStatus() == CardStatus::Assigned) && !card->isUnderConstruction())
        {
            Action a = guiCard.onClick(m_currentState);
            doGUIAction(a);
        }
    }
            
    setState(m_currentState); 
}


void GUIState_Play::sUserInput()
{
    // if an AI move is being carried out, don't allow any input
    if (m_doingAIMove) { return; }

    sf::Event event;
    int menuIndexChange = 0;
    PlayerID player = m_currentState.getActivePlayer();
    bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    while (m_game.window().pollEvent(event))
    {
        // this event triggers when the window is closed
        if (event.type == sf::Event::Closed) { m_game.quit(); }

        // this event is triggered when a key is pressed
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.code)
            {
                case sf::Keyboard::Escape:  { m_game.popState(); break; }
                case sf::Keyboard::Tab:     { toggleBool(m_drawBaseSetCards); break; }
                case sf::Keyboard::Q:       { activateWorkers(); break; }
                case sf::Keyboard::Space:   { endCurrentPhase(); break; }
                case sf::Keyboard::A:       { buyCardByName("Animus", shift); break; }
                case sf::Keyboard::D:       { buyCardByName("Drone", shift); break; }
                case sf::Keyboard::E:       { buyCardByName("Engineer", shift); break; }
                case sf::Keyboard::B:       { buyCardByName("Blastforge", shift); break; }
                case sf::Keyboard::C:       { buyCardByName("Conduit", shift); break; }
                case sf::Keyboard::F:       { buyCardByName("Forcefield", shift); break; }
                case sf::Keyboard::G:       { buyCardByName("Gauss Cannon", shift); break; }
                case sf::Keyboard::S:       { buyCardByName("Steelsplitter", shift); menuIndexChange = 1; break; }
                case sf::Keyboard::T:       { buyCardByName("Tarsier", shift); break; }
                case sf::Keyboard::R:       { buyCardByName("Rhino", shift); break; }
                case sf::Keyboard::W:       { buyCardByName("Wall", shift); menuIndexChange = -1; break; }
                case sf::Keyboard::Z:       { rewindToPreviousState(); break; }
                case sf::Keyboard::M:       { toggleBool(m_drawMouseOver); break; }
                case sf::Keyboard::X:       { toggleBool(m_drawPotentials); break; }
                case sf::Keyboard::Tilde:   { toggleBool(m_drawDebugInfo); break; }
                case sf::Keyboard::Return:  { handleAIMenu(); break; }
                case sf::Keyboard::Up:      { menuIndexChange = -1; break; }
                case sf::Keyboard::Down:    { menuIndexChange =  1; break; }

                default:                    { break; }
            }

            // change the ai menu selected item based on the input above
            m_selectedPlayer[player] = (m_selectedPlayer[player] + (m_drawAIMenu ? menuIndexChange : 0));
            if (m_selectedPlayer[player] < 0) { m_selectedPlayer[player] += m_players[player].size(); }
            else { m_selectedPlayer[player] = m_selectedPlayer[player] % m_players[player].size(); }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            auto mouse = m_view.windowToWorld(Vec2(event.mouseButton.x, event.mouseButton.y));

            // happens when the left mouse button is pressed
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                m_view.stopScroll();

                // determine which element of the gui was clicked, if any
                GUICard * guiCard = getClickedCard(mouse.x, mouse.y);
                GUICardBuyable * cardBuyable = guiCard ? NULL : getClickedCardBuyable(mouse.x, mouse.y);
                bool shift = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

                // if a card was clicked
                if (guiCard)
                {
                    Action a = guiCard->onClick(m_currentState);
                    a.setShift(shift);
                    if (m_currentState.isLegal(a)) { doGUIAction(a); }
                }
                // if a buyable card pane was clicked
                else if (cardBuyable != NULL)
                {
                    Action a = cardBuyable->onClick(m_currentState.getActivePlayer(), m_currentState.getActivePhase());
                    a.setShift(shift);
                    if (m_currentState.isLegal(a)) { doGUIAction(a); }
                }
            }

            // happens when the right mouse button is pressed
            if (event.mouseButton.button == sf::Mouse::Right)
            {
                m_drag = { event.mouseButton.x, event.mouseButton.y };
                m_view.stopScroll();
            }
        }

        // happens when the mouse button is released
        if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left)  { }
            if (event.mouseButton.button == sf::Mouse::Right) { m_drag = { -1, -1 }; }
        }

        if (event.type == sf::Event::MouseWheelMoved)
        {
            double zoom = 1.0 - (0.2 * event.mouseWheel.delta);
            m_view.zoomTo(zoom, Vec2(event.mouseWheel.x, event.mouseWheel.y));
        }

        // happens whenever the mouse is being moved
        if (event.type == sf::Event::MouseMoved)
        {
            auto world = m_view.windowToWorld(Vec2(event.mouseMove.x, event.mouseMove.y));
            m_mouseWorld = sf::Vector2f(world.x, world.y);
            m_mouseOverCard = getClickedCard(world.x, world.y);
            m_mouseOverCardBuyable = getClickedCardBuyable(world.x, world.y);

            // dragging with rmb
            if (m_drag.x != -1)
            {
                auto prev = m_view.windowToWorld(m_drag);
                auto curr = m_view.windowToWorld({ event.mouseMove.x, event.mouseMove.y });
                auto scroll = prev - curr;
                m_view.scroll(prev - curr);
                m_drag = { event.mouseMove.x, event.mouseMove.y };
            }

        }
    }
}

void GUIState_Play::toggleBool(bool & val)
{
    val = !val;
}

void GUIState_Play::buyCardByName(const std::string & name, bool shift)
{
    if (CardTypes::CardTypeExists(name))
    {
        Action buy(m_currentState.getActivePlayer(), ActionTypes::BUY, CardTypes::GetCardType(name).getID());
        buy.setShift(shift);
        doGUIAction(buy);
    }
}

GUICard * GUIState_Play::getClickedCard(const int x, const int y)
{
    GUICard * clicked = nullptr;
    int maxCardLayer = -1;
    for (CardID c(0); c<m_guiCards.size(); ++c)
    {
        if (m_guiCards[c].isClicked(x, y) && (!clicked || m_guiCards[c].getLayer() > maxCardLayer))
        {
            clicked = &m_guiCards[c];
            maxCardLayer = m_guiCards[c].getLayer();
        }
    }
    
    return clicked;
}

GUICardBuyable * GUIState_Play::getClickedCardBuyable(const int x, const int y)
{
    GUICardBuyable * clicked = nullptr;
    int maxCardLayer = -1;
    for (CardID c(0); c<m_guiCardsBuyable.size(); ++c)
    {
        if (m_guiCardsBuyable[c].isClicked(x, y) && (!clicked || m_guiCardsBuyable[c].getLayer() > maxCardLayer))
        {
            clicked = &m_guiCardsBuyable[c];
            maxCardLayer = m_guiCardsBuyable[c].getLayer();
        }
    }

    return clicked;
}

// draws the large scale interface elements
void GUIState_Play::drawInterface()
{
    const sf::Vector2f BuyablePaneSize(200, 0);
    GUITools::DrawTexturedRect({0, 0}, sf::Vector2f(m_game.window().getSize()), "TexBG", sf::Color::White, &m_game.window()); // board bg
    GUITools::DrawRect(sf::Vector2f(0,0), sf::Vector2f(BuyablePaneSize.x, m_game.window().getSize().y), sf::Color::Black, &m_game.window()); // buy pane bg
    sf::Vector2f p1Origin = BuyablePaneSize;
    sf::Vector2f p2Origin(BuyablePaneSize.x, m_game.window().getSize().y/2);
    GUITools::DrawRect(p2Origin, sf::Vector2f(m_game.window().getSize().x-BuyablePaneSize.x, 3), sf::Color(127, 127, 127, 127), &m_game.window()); // horizontal board sep
    GUITools::DrawRect(p1Origin, sf::Vector2f(3, m_game.window().getSize().y), sf::Color(127, 127, 127, 64), &m_game.window()); // vertical buy sep
}

// draws resources, attack amounts, phase, etc
void GUIState_Play::drawInformation()
{
    // draw resource
    const sf::Vector2f BuyablePaneSize(200, 0);
    sf::Vector2f iconSize(32, 32);
    sf::Vector2f bigIconSize(64, 64);
    sf::Vector2f numberSize(iconSize.x/2, iconSize.y/2);
    sf::Vector2f diffSize((iconSize.x - numberSize.x) / 2, (iconSize.y - numberSize.y) / 2);
    sf::Vector2f buffer(10,-10);
    sf::Vector2f p1Origin = BuyablePaneSize;
    sf::Vector2f p2Origin(BuyablePaneSize.x, m_game.window().getSize().y/2);
    sf::Vector2f origin[2] = {BuyablePaneSize + sf::Vector2f(10, -10) + sf::Vector2f(0, m_game.window().getSize().y - iconSize.y), BuyablePaneSize + sf::Vector2f(10, 10)};

    int iconNum = 0;
    sf::Color white(255, 255, 255, 255);
    sf::Color white2(255, 255, 255, 127);
    sf::Color bg(0, 0, 0, 127);

    for (PlayerID player=0; player<2; ++player)
    {
        GUITools::DrawMana(m_currentState.getResources(player), origin[player], iconSize, numberSize, sf::Vector2f(10, 0), true, &m_game.window());
    }

    int phase = m_currentState.getActivePhase();
    PlayerID player = m_currentState.getActivePlayer();
    sf::Vector2f attackSize(300, 300);
    sf::Vector2f numSize(80, 80);
    sf::Vector2f midPoint(m_game.window().getSize().x/2 + BuyablePaneSize.x/2, m_game.window().getSize().y/2);
    if (phase == Phases::Breach || phase == Phases::Defense)
    {        
        HealthType attack = m_currentState.getAttack(player);
        if (phase == Phases::Defense)
        {
            midPoint = midPoint + sf::Vector2f(0, player ? midPoint.y/2 : -midPoint.y/2);
            attack = m_currentState.getAttack(m_currentState.getEnemy(player));
            GUITools::DrawTexturedRect(midPoint - sf::Vector2f(attackSize.x/2, attackSize.y/2), sf::Vector2f(attackSize.x, attackSize.y), "TexAttackBig", white, &m_game.window());
        }
        else
        {
            midPoint = midPoint + sf::Vector2f(0, player ? -midPoint.y/2 : midPoint.y/2);
            GUITools::DrawTexturedRect(midPoint - sf::Vector2f(attackSize.x/2, attackSize.y/2), sf::Vector2f(attackSize.x, attackSize.y), "TexAttackBigRed", white, &m_game.window());
        }

        GUITools::DrawTexturedRect(midPoint - sf::Vector2f(numSize.x/2, numSize.y/2), sf::Vector2f(numSize.x/2, numSize.y/2), std::to_string(attack), white, &m_game.window());
    }

    // print status message
    sf::Vector2f space = m_currentState.getActivePlayer() != 0 ? sf::Vector2f(m_game.window().getSize().x/2 - 200, 8) : sf::Vector2f(m_game.window().getSize().x/2 - 200, m_game.window().getSize().y - 24);
    std::string status = "";

    if (m_currentState.isLegal(Action(m_currentState.getActivePlayer(), ActionTypes::END_PHASE, 0)))
    {
        status = m_currentState.getActivePhase() == Phases::Action ? "ACTION PHASE - PRESS SPACE TO END PHASE" : "PRESS SPACE TO CONFIRM END PHASE";
    }
    else
    {
        switch (m_currentState.getActivePhase())
        {
            case Phases::Defense: { status = "DEFENSE PHASE - ASSIGN BLOCKERS"; break; }
            case Phases::Breach:  { status = "BREACH PHASE - ASSIGN BREACH"; break; }
        }
    }

    // draw attack and defense potentials
    if (m_drawPotentials && !m_currentState.isTargetAbilityCardClicked())
    {
        auto wSize = m_game.window().getSize();
        sf::Vector2f atkPos[2] = { sf::Vector2f(BuyablePaneSize.x + 20, wSize.y/2 + 20), sf::Vector2f(BuyablePaneSize.x + 20, wSize.y/2 - 70) };
        HealthType def[2] = { m_currentState.getTotalAvailableDefense(0), m_currentState.getTotalAvailableDefense(1) };

        for (PlayerID p(0); p < 2; ++p)
        {
            std::stringstream ss;
            ss << "Defense: " << def[p] << "\n";
        
            HealthType atk = 0;
            if (m_currentState.getActivePlayer() == p)
            {
                atk = m_currentState.getAttack(p);   
            }
            else
            {
                GameState atkState(m_currentState);
                AITools::PredictEnemyNextTurn(atkState);
                atk = atkState.getAttack(p);
            }

            ss << "Attack:  " << atk;

            GUITools::DrawString(atkPos[p], ss.str(), sf::Color::White, &m_game.window(), 24);
        }
    }

    GUITools::DrawString(space, status, sf::Color::White, &m_game.window(), 16);
    int spacing = 15;
    int top = 140;

    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top), "Enter: AI Menu" , sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 1*spacing),  "ESC:   Main Menu", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 2*spacing),  "TAB:   Buy Pane", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 3*spacing),  "Q:     Tap Drones", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 4*spacing),  "Z:     Undo Action", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 5*spacing),  "X:     Toggle Atk/Def", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 6*spacing),  "M:     Toggle Mouseover", sf::Color(127, 127, 127), &m_game.window());
    GUITools::DrawString(sf::Vector2f(5, m_game.window().getSize().y - top + 7*spacing),  "Tilde: Toggle Debug", sf::Color(127, 127, 127), &m_game.window());
}

// draws all cards and buyable cards
void GUIState_Play::drawCards()
{
    // draw the live cards
    for (size_t i=0; i<m_guiCards.size(); i++)
    {
        m_guiCards[i].draw(i, m_currentState, false);
    }
    // draw the buyable cards
    for (CardID i(0); i<m_guiCardsBuyable.size(); ++i)
    {
        if (m_drawBaseSetCards && CardTypes::IsBaseSet(m_guiCardsBuyable[i].getType()))
        {
            m_guiCardsBuyable[i].draw(i, m_currentState);
        }

        if (!m_drawBaseSetCards && !CardTypes::IsBaseSet(m_guiCardsBuyable[i].getType()))
        {
            m_guiCardsBuyable[i].draw(i, m_currentState);
        }
    }
}

// draws debug info output from ai players
void GUIState_Play::drawDebugInfo()
{
    if (!m_drawDebugInfo) { return; }
    sf::Vector2f origins[2] = { sf::Vector2f(m_game.window().getSize().x - 300, (m_game.window().getSize().y / 2) + 20), sf::Vector2f(m_game.window().getSize().x - 300, 20)};

    std::stringstream ss[2];

    for (PlayerID p(0); p<2; ++p)
    {
        ss[p] << "Will Score : " << Eval::WillScoreSum(m_currentState, p) << "\n\n";
        ss[p] << m_aiDescription[p];

        GUITools::DrawString(origins[p], ss[p].str(), sf::Color::White, &m_game.window());
    }
}

void GUIState_Play::drawTargetAbility()
{
    if (m_currentState.isTargetAbilityCardClicked() && !m_doingAIMove)
    {   
        GUICard * targetAbilityCard = NULL;

        // find the guicard corresponding to the target ability card that was clicked
        for (size_t i(0); i < m_guiCards.size(); ++i)
        {
            CardID guiCardID = m_guiCards[i].getCard()->getID();
            CardID targetAbilityCardID = m_currentState.getTargetAbilityCardClicked().getID();
            if (guiCardID == targetAbilityCardID)
            {
                targetAbilityCard = &m_guiCards[i];
                break;
            }
        }

        if (targetAbilityCard)
        {
            GUITools::DrawLine(targetAbilityCard->pos() + sf::Vector2f(55, 55), m_mouseWorld, sf::Color::Red, &m_game.window());
        }
    }
}

void GUIState_Play::drawMouseOverPanes()
{
    if (!m_drawMouseOver) { return; }

    // draw mouseover box
    if (m_mouseOverCard)
    {
        GUITools::DrawMouseOverPane(m_mouseOverCard->getCard()->getType(), m_mouseOverCard->pos() + sf::Vector2f(120, 0), m_mouseOverCard->getCard(), &m_game.window());
    }

    if (m_mouseOverCardBuyable)
    {
        GUITools::DrawMouseOverPane(m_mouseOverCardBuyable->getType(), m_mouseOverCardBuyable->pos() + sf::Vector2f(210, 0), nullptr, &m_game.window());
    }
}

void GUIState_Play::handleAIMenu()
{
    // ai menu button clicked, so toggle it
    m_drawAIMenu = !m_drawAIMenu;
    if (m_drawAIMenu) { return; }

    // do ai move: get the PlayerPtr associated with the currently highlighted player
    PlayerPtr ptr = AIParameters::Instance().getPlayer(m_currentState.getActivePlayer(), m_selectedPlayerName[m_currentState.getActivePlayer()]);
    Move move;
    ptr->getMove(m_currentState, move);
    m_aiDescription[m_currentState.getActivePlayer()] = ptr->getDescription();

    // test code I wanna leave here
    //bool shouldResign = shouldResign = AITools::PlayerShouldResign(m_currentState, player);
    //std::cout << move.toString() << "\n";
    //std::cout << "Player Resign: " << (shouldResign ? "true" : "false") << "\n";
    // get the click string associated with that move & convert that click string back to a move to test (used by old ai for testing)
    //const std::string moveClickString = AITools::GetClickString(move, m_currentState);
    //const Move & convertedMove = AITools::GetMoveFromClickString(moveClickString, m_currentState.getActivePlayer(), m_currentState);
    //std::cout << AITools::GetClickString(convertedMove, m_currentState);

    doGUIMove(move, 200);
}

void GUIState_Play::drawAIMenu()
{
    if (!m_drawAIMenu) { return; }

    int fontSize = 18;
    int fontVertSpacing = 20;
    sf::Vector2f size(700, 40 + m_players[0].size() * fontVertSpacing + 100);
    auto windowSize = m_game.window().getSize();

    sf::Vector2f pos(windowSize.x/2 - size.x/2, windowSize.y/2 - size.y/2);
    const PlayerID player(m_currentState.getActivePlayer());
    GUITools::DrawRect(pos, size, sf::Color(0, 0, 0, 230), &m_game.window());
    std::stringstream ss;
    ss << "Select AI to Move for Player ID " << (int)m_currentState.getActivePlayer() << "  " << (player ? "(Top)" : "(Bottom)");
    GUITools::DrawString(pos + sf::Vector2f(25, 20), ss.str(), sf::Color::Yellow, &m_game.window(), fontSize);

    size_t index = 0;
    std::map<std::string, PlayerPtr>::iterator it;
    for (it = m_players[player].begin(); it != m_players[player].end(); it++)
    {
        std::stringstream ss;
        ss << (index < 10 ? "0" : "") << (index)<< ": " << (*it).first;

        if (index == m_selectedPlayer[player])
        {
            GUITools::DrawString(pos + sf::Vector2f(25, 60 + index * fontVertSpacing), ss.str(), player ? sf::Color::Red : sf::Color::Green, &m_game.window(), fontSize);
            m_selectedPlayerName[player] = (*it).first;
                
            std::string header = it->first + " Description:\n";
            GUITools::DrawString(pos + sf::Vector2f(330, 60), header, player ? sf::Color::Red : sf::Color::Green, &m_game.window(), fontSize);
            GUITools::DrawString(pos + sf::Vector2f(330, 80), it->second->getDescription(), sf::Color::White, &m_game.window(), fontSize);
        }
        else
        {
            GUITools::DrawString(pos + sf::Vector2f(25, 60 + index * fontVertSpacing), ss.str(), sf::Color::White, &m_game.window(), fontSize);
        }

        ++index;
    }
}

// renders the scene
void GUIState_Play::sRender()
{
    // switch to world view to draw things in world coordinates
    m_game.window().clear();
    m_game.window().setView(m_view.getSFMLView());
        
    // render all the relevant game information
    drawInterface();
    drawCards();
    drawInformation();
    drawAIMenu();
    drawDebugInfo();
    drawTargetAbility();
    drawMouseOverPanes();
    

    // swap the buffers and draw the frame
    m_game.window().display();
}