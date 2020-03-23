#include "GUICard.h"
#include "Assets.h"
#include "GUITools.h"

using namespace Prismata;

const float scale = 1.0f;
const sf::Vector2f OriginalCardSize        = {110, 110};
sf::Vector2f CardSize                      = sf::Vector2f(OriginalCardSize.x * scale, OriginalCardSize.y * scale);
const sf::Vector2f BorderSize              = sf::Vector2f(1, 1);
const sf::Vector2f TextureOffset           = sf::Vector2f(CardSize.x / 5, CardSize.y / 5);
const sf::Vector2f PortraitSize            = sf::Vector2f(CardSize - TextureOffset - sf::Vector2f(5, 5));
const sf::Vector2f CardNameOffset          = sf::Vector2f(10, 4);
const sf::Vector2f BlackBoxOffset          = sf::Vector2f(CardSize.x / 5, CardSize.y - 20);
const sf::Vector2f ADOffset                = CardSize - sf::Vector2f(5, 5);
const sf::Vector2f BuyCostOffset           = sf::Vector2f(CardSize.x - 5, 16);
const sf::Vector2f StatusIconSize          = sf::Vector2f(CardSize.x / 5, CardSize.y / 5);
const sf::Vector2f StatusIconOffset        = sf::Vector2f(CardSize.x / 25, CardSize.y / 9);
const sf::Vector2f AttackIconOffset        = sf::Vector2f(TextureOffset.x + 5, CardSize.y - 25);
const sf::Vector2f DefenseIconOffset       = sf::Vector2f(CardSize.y - 25, AttackIconOffset.y);

const sf::Color White(255, 255, 255, 255);
const sf::Color Blue(0, 0, 255, 255);
const sf::Color BorderWhite(255, 255, 255, 255);
const sf::Color WhiteCardName(255, 255, 255, 255);
const sf::Color WhiteFade(255, 255, 255, 200);
const sf::Color Grey(200, 200, 200, 200);
const sf::Color DarkGrey(120, 120, 120, 255);
const sf::Color Red(255, 0, 0, 255);
const sf::Color CardImage(255, 255,255, 255);

GUICard::GUICard()
{

}

GUICard::GUICard(const Card & card, const sf::Vector2f & p, sf::RenderWindow & window)
    : m_window(&window)
    , m_card(&card)
    , m_pos(p)
    , m_layer(0)
{

}

void GUICard::SetCardSize(sf::Vector2f size)
{
    CardSize = size;
}

sf::Vector2f GUICard::GetCardSize()
{
    return CardSize;
}

const int GUICard::getLane() const
{
    const CardType type = m_card->getType();

    if (type.isFrontline())
    {
        return 0;
    }

    if (type.hasAbility() || type.hasTargetAbility())
    {
        return 1;
    }
    else
    {
        if (type.canBlock(false))
        {
            return 0;
        }
        else
        {
            return 2;
        }
    }
}

const bool GUICard::operator < (const GUICard & rhs) const
{
    if (m_card->getType() == rhs.m_card->getType())
    {
        if (m_card->getConstructionTime() == rhs.m_card->getConstructionTime())
        {
            return m_card->getID() < rhs.m_card->getID();
        }
        else
        {
            return m_card->getConstructionTime() > rhs.m_card->getConstructionTime();
        }
    }
    else
    {
        return m_card->getType() < rhs.m_card->getType();
    }   
}

const Card * GUICard::getCard()
{
    return m_card;
}

sf::Vector2f GUICard::pos() const
{
    return m_pos;
}

void GUICard::setPosition(const sf::Vector2f & pos)
{
    m_pos = pos;
}

const int GUICard::getLayer() const
{
    return m_layer;
}

const bool GUICard::isClicked(int x, int y) const
{
    return (x >= pos().x) && (x <= pos().x + CardSize.x) && (y >= pos().y) && (y <= pos().y + CardSize.y);
}

Action GUICard::onClick(const GameState & currentState)
{
    return currentState.getClickAction(*m_card);
}


void GUICard::draw(const int layer, const GameState & state, bool isFirstInStack)
{
    if (m_pos == sf::Vector2f(-1,-1))
    {
        return;
    }

    PlayerID activePlayer = state.getActivePlayer();
    PlayerID inactivePlayer = state.getEnemy(activePlayer);

    m_layer = layer;

    std::stringstream ad;
    ad << (int)m_card->getType().getAttack() << " / " << (int)m_card->currentHealth();

    std::stringstream id;
    id << m_card->getType().getID();

    sf::Vector2f statusPos = pos() + StatusIconOffset;
        
    bool canBlock = m_card->canBlock();
    bool constr = m_card->isUnderConstruction();

    Action clickAction = onClick(state);
    bool actionLegal = state.isLegal(clickAction);

    // Determine the background texture to draw
    std::string       backgroundTexture = "TexCardBGDefault";
    if (!canBlock)  { backgroundTexture = "TexCardBGAssigned"; }
    if (constr)     { backgroundTexture = "TexCardBGConstruction"; }

    if (actionLegal && ((clickAction.getType() == ActionTypes::ASSIGN_FRONTLINE) || (clickAction.getType() == ActionTypes::ASSIGN_BREACH)))
    {
        backgroundTexture = "TexCardBGBreach";
    }

    if (m_card->isDead())        { backgroundTexture = "TexCardBGDead"; }

    bool drawProps = true;

    // card background
    GUITools::DrawTexturedRect(pos(), CardSize, backgroundTexture, White, m_window);

    // card image
    GUITools::DrawTexturedRect(pos() + TextureOffset, PortraitSize, m_card->getType().getUIName(), constr ? WhiteFade : White, m_window);
        
    // card name 
    if (drawProps)
    {
        GUITools::DrawString(pos() + CardNameOffset + sf::Vector2f(TextureOffset.x, 0), m_card->getType().getUIName().substr(0, std::min((size_t)10, m_card->getType().getUIName().length())), White, m_window);
    }
        
    // construction time
    sf::Vector2f topLeftStatusPos = pos() + sf::Vector2f(-2, 13);
    if (constr)
    {
        GUITools::DrawTexturedRect(pos(), CardSize, "TexStatusConstruction", White, m_window);
        GUITools::DrawTexturedRect(topLeftStatusPos, sf::Vector2f(14,14), std::to_string(m_card->getConstructionTime()), White, m_window); 
    }
    else if (m_card->canBlock())
    {
        GUITools::DrawTexturedRect(pos(), CardSize, m_card->isSellable() ? "TexStatusGoldShield" : "TexStatusBlueShield", White, m_window);
    }
    else if (m_card->getType().canBlock(false))
    {
        GUITools::DrawTexturedRect(pos(), CardSize, "TexStatusWhiteShield", White, m_window);
    }

    if (drawProps)
    {
        int attack = (int)m_card->getType().getAttack();
        if (attack > 0)
        {
            GUITools::DrawIconAndText(pos() + AttackIconOffset, StatusIconSize, "TexAttack", std::to_string(attack), White, m_window);
        }

        if (m_card->getType().isFragile())
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);

            GUITools::DrawIconAndText(statusPos, StatusIconSize, "TexStatusHP", std::to_string(m_card->currentHealth()), White, m_window);
        }
        else
        {
            int defense = (int)m_card->currentHealth();
            if (defense > 0)
            {
                GUITools::DrawIconAndText(pos() + DefenseIconOffset, StatusIconSize, "TexDefense", std::to_string(defense), White, m_window);
            }
        }

        if (m_card->getType().usesCharges())
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);
            GUITools::DrawIconAndText(statusPos, StatusIconSize, "TexStatusCharge" + std::to_string(m_card->getCurrentCharges()), std::to_string(m_card->getCurrentCharges()), White, m_window);
        }

        if (m_card->getCurrentLifespan() > 0 && !constr)
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);
            GUITools::DrawIconAndText(statusPos, StatusIconSize, "TexStatusDoom", std::to_string(m_card->getCurrentLifespan()), White, m_window);
        }

        if (m_card->getCurrentDelay() > 0 && !constr)
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);
            GUITools::DrawIconAndText(statusPos, StatusIconSize, "TexStatusDelay", std::to_string(m_card->getCurrentDelay()), White, m_window);
        }

        if (m_card->currentChill() > 0 && !constr)
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);
            GUITools::DrawIconAndText(statusPos, StatusIconSize, "TexStatusTap", std::to_string(m_card->currentChill()), White, m_window);
        }

        if (m_card->getType().isFrontline() && !constr)
        {
            statusPos = statusPos + sf::Vector2f(0, StatusIconSize.y + 3);
            GUITools::DrawTexturedRect(statusPos, StatusIconSize, "TexStatusUndefendable", White, m_window);
        }
    }


    if (actionLegal)
    {
        sf::Color borderColor = sf::Color(BorderWhite.r, BorderWhite.g, BorderWhite.b, 255);
        GUITools::DrawTexturedRect(pos(), CardSize, "TexCardBGBorderGreen", borderColor, m_window);
    }
}

bool GUICardNewSort::operator() (GUICard & a, GUICard & b)
{
    CardID aTypeID = a.getCard()->getType().getID();
    CardID bTypeID = b.getCard()->getType().getID();

    if (aTypeID == bTypeID)
    {
        return a.getCard()->getID() < b.getCard()->getID();
    }
    else
    {
        return aTypeID < bTypeID;
    }
}
