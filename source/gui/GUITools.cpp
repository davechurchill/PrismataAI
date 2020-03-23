#include "GUITools.h"
#include "Assets.h"
#include "Action.h"

namespace Prismata
{
namespace GUITools
{
    void DrawRect(sf::Vector2f tl, sf::Vector2f size, sf::Color color, sf::RenderWindow * window)
    {
        sf::RectangleShape rect;
        rect.setPosition(tl);
        rect.setSize(size);
        rect.setFillColor(color);
        window->draw(rect);
    }

    void DrawTexturedRect(sf::Vector2f tl, sf::Vector2f size, const std::string & tex, sf::Color color, sf::RenderWindow * window)
    {
        sf::RectangleShape rect;
        rect.setPosition(tl);
        rect.setSize(size);
        rect.setTexture(&Assets::Instance().getTexture(tex));
        rect.setFillColor(color);
        window->draw(rect);
    }

    void DrawString(sf::Vector2f p, const std::string & str, sf::Color color, sf::RenderWindow * window, int size)
    {
        sf::Text text;
        text.setString(str);
        text.setFont(Assets::Instance().getFont("Consolas"));
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setPosition(p);
        window->draw(text);
    }

    void DrawIconAndText(const sf::Vector2f & tl, const sf::Vector2f & size, const std::string & tex1, const std::string & tex2, sf::Color color, sf::RenderWindow * window)
    {
        sf::Vector2f statusNumPos = tl + sf::Vector2f(-size.x*0.15, size.y*0.4);
        DrawTexturedRect(tl, size, tex1, color, window);
        DrawTexturedRect(statusNumPos, sf::Vector2f(size.x*0.65, size.y*0.65), tex2, color, window); 
    }

    void DrawLine(const sf::Vector2f & p1, const sf::Vector2f & p2, sf::Color color, sf::RenderWindow * window)
    {
        sf::VertexArray lines(sf::LinesStrip, 2);
        lines[0].position = p1; lines[0].color = color;
        lines[1].position = p2; lines[1].color = color;
        window->draw(lines);
    }

    void DrawMana(const Resources & resource, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window)
    {
        sf::Vector2f diffSize((iconSize.x - numberSize.x) / 3, (iconSize.y - numberSize.y) / 3);

        int iconNum = 0;
        sf::Color white(255, 255, 255, 255);
        
        sf::Vector2f iconDiff(0, 0);

        static std::vector<std::string> ManaStrings = { "TexGold", "TexEnergy", "TexBlue", "TexRed", "TexGreen", "TexAttack" };
        for (size_t m(0); m < Resources::NumTypes; ++m)
        {
            if (drawZeros || (!drawZeros && resource.amountOf(m) > 0))
            {
                DrawTexturedRect(origin + iconDiff, iconSize, ManaStrings[m], white, window); 
                DrawTexturedRect(origin + iconDiff + diffSize + sf::Vector2f(12, 12), numberSize + diffSize, std::to_string(resource.amountOf(m)%21), white, window); 
                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }
    }

    void DrawBuyCost(const CardType cardType, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window)
    {
        sf::Vector2f diffSize((iconSize.x - numberSize.x) / 2, (iconSize.y - numberSize.y) / 2);
        Resources cost = cardType.getBuyCost();

        int iconNum = 0;
        sf::Color white(255, 255, 255, 255);
        int resourceTextureOffset = 221;
        int numberTextureOffset = 200;

        sf::Vector2f iconDiff(0, 0);

        static std::vector<std::string> ManaStrings = { "TexGold", "TexEnergy", "TexBlue", "TexRed", "TexGreen", "TexAttack" };
        for (size_t m(0); m < Resources::NumTypes; ++m)
        {
            if (drawZeros || (!drawZeros && cost.amountOf(m) > 0))
            {
                GUITools::DrawTexturedRect(origin + iconDiff, iconSize, ManaStrings[m], white, window); 
                GUITools::DrawTexturedRect(origin + iconDiff + diffSize, numberSize + diffSize, std::to_string((cost.amountOf(m)%21)), white, window); 
                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }

        for (size_t i(0); i<cardType.getBuySac().size(); ++i)
        {
            CardType sacType = cardType.getBuySac()[i].getType();
            GUITools::DrawIconAndText(origin + iconDiff, iconSize, sacType.getUIName(), std::to_string(cardType.getBuySac()[i].getMultiple()), white, window); 
            iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
        }
    }

    void DrawMouseOverPane(const CardType type, const sf::Vector2f & pos, const Card * card, sf::RenderWindow * window)
    {
        sf::Vector2f iconSize(32,32);
        sf::Vector2f halfIconSize(iconSize.x/2, iconSize.y/2);
        sf::Vector2f paneSize(380, 260);
        sf::Vector2f borderSize(5, 5);
        sf::Vector2f borderSize2(10, 10);
        sf::Vector2f iconBuffer(6,0);
        sf::Color black(0, 0, 0, 220);
        sf::Color fade(200, 200, 200, 40);
        sf::Color border(255, 255, 255, 255);
        sf::Color white(255, 255, 255, 255);
        sf::Color bg(0, 0, 0, 220);
        sf::Color fadeCost(200, 200, 200, 150);

        sf::Vector2f namePlateSize(paneSize.x-borderSize.x*2, 30);

        sf::Vector2f columnBuffer(iconSize.x + 30, 0);
        sf::Vector2f statusIcon(pos + borderSize + sf::Vector2f(10,10+namePlateSize.y));
        sf::Vector2f cost(statusIcon + columnBuffer);
        sf::Vector2f statusIconBuffer(0, iconSize.y + 10);
        sf::Vector2f propStatus(pos + sf::Vector2f(paneSize.x - borderSize.x - iconSize.x - 10, borderSize.y + 10 + namePlateSize.y));

        DrawRect(pos, paneSize, border, window);
        DrawRect(pos + borderSize, paneSize - borderSize2, bg, window);
        DrawRect(pos + borderSize, namePlateSize, black, window);
        DrawString(pos + borderSize + sf::Vector2f(8,0), type.getUIName(), white, window, 24);

        sf::Vector2f typePos(pos + borderSize + sf::Vector2f(paneSize.x - paneSize.y, 0));
        DrawTexturedRect(typePos, sf::Vector2f(paneSize.y, paneSize.y) - borderSize2, type.getUIName(), fade, window);

        
        DrawTexturedRect(statusIcon, iconSize, "TexCost", white, window);
        DrawBuyCost(type, cost, iconSize, halfIconSize, iconBuffer, false, window);

        statusIcon = statusIcon + statusIconBuffer;

        if (card)
        {
            std::stringstream ss;
            ss << "ID: " << (int)(card->getID()) << " ";
            ss << card->toJSONString(true);
            DrawString(pos + sf::Vector2f(175, 55), ss.str(), white, window);
        }

        
        if (type.isFragile())
        {
            DrawIconAndText(propStatus, iconSize,"TexStatusHP", std::to_string(type.getStartingHealth()), white, window);
            propStatus = propStatus + statusIconBuffer;
        }
        else
        {
            DrawIconAndText(propStatus, iconSize, "TexDefense", std::to_string(type.getStartingHealth()), white, window);
            propStatus = propStatus + statusIconBuffer;
        }

        if (type.usesCharges())
        {
            DrawIconAndText(propStatus, iconSize, "TexStatusCharge" + std::to_string(type.getStartingCharge()), std::to_string(type.getStartingCharge()), white, window);
            propStatus = propStatus + statusIconBuffer;
        }

        if (type.getLifespan() > 0)
        {
            GUITools::DrawIconAndText(propStatus, iconSize, "TexStatusDoom", std::to_string(type.getLifespan()), white, window);
            propStatus = propStatus + statusIconBuffer;
        }

        if (type.isFrontline())
        {
            DrawTexturedRect(propStatus, iconSize, "TexStatusUndefendable", white, window);
            propStatus = propStatus + statusIconBuffer;
        }

        
        sf::Vector2f buildPos(statusIcon);

        if (type.getConstructionTime() > 0 || type.canBlock(false) || type.getBuyScript().hasEffect())
        {
            DrawTexturedRect(buildPos, iconSize, "TexOnBuy", white, window);
            buildPos = buildPos + columnBuffer;
        }

        if (type.getConstructionTime() > 0)
        {
            DrawIconAndText(buildPos, iconSize, "TexClock", std::to_string(type.getConstructionTime()), white, window);
            buildPos = buildPos + sf::Vector2f(iconSize.x, 0) + iconBuffer;
        }
    
        if (type.canBlock(false) && type.getConstructionTime() == 0)
        {
            DrawTexturedRect(buildPos,iconSize, "TexDefenseBig", white, window);
            buildPos = buildPos + sf::Vector2f(iconSize.x, 0) + iconBuffer;
        }
        
        if (type.getBuyScript().hasEffect())
        {
            DrawScriptEffect(type.getBuyScript(), buildPos, iconSize, halfIconSize, iconBuffer, false, window);
        }

        statusIcon = statusIcon + statusIconBuffer;

        sf::Vector2f begin(statusIcon);
        bool addStatus = false;
        if (type.canBlock(false))
        {
            DrawTexturedRect(begin, iconSize, "TexStartTurn", white, window);
            DrawTexturedRect(begin + columnBuffer, iconSize, "TexDefenseBig", white, window);
            addStatus = true;
        }

        if (type.hasBeginOwnTurnScript() || type.getHealthGained() > 0)
        {
            if (!addStatus)
            {
                DrawTexturedRect(begin, iconSize, "TexStartTurn", white, window);
            }
            else
            {
                begin = begin + sf::Vector2f(iconSize.x, 0) + iconBuffer;
            }

            if (type.getHealthGained() > 0)
            {
                DrawIconAndText(begin + columnBuffer, iconSize, "TexStatusHP", std::to_string(type.getHealthGained()), white, window);
                begin = begin + sf::Vector2f(iconSize.x, 0) + iconBuffer;
            }

            if (type.getBeginOwnTurnScript().getDelay() > 0)
            {
                DrawIconAndText(begin + columnBuffer, iconSize, "TexStatusDelay", std::to_string(type.getBeginOwnTurnScript().getDelay()), white, window);
                begin = begin + sf::Vector2f(iconSize.x, 0) + iconBuffer;
            }

            GUITools::DrawScriptEffect(type.getBeginOwnTurnScript(), begin + columnBuffer, iconSize, halfIconSize, iconBuffer, false, window);
            addStatus = true;
        }

        if (addStatus)
        {
            statusIcon = statusIcon + statusIconBuffer;
        }

        if (type.hasAbility() || type.hasTargetAbility())
        {
            sf::Vector2f p(statusIcon);
            sf::Vector2f plusSize(iconSize.x/3, iconSize.y/3);
            sf::Vector2f plus(p + sf::Vector2f(iconSize.x + 10, iconSize.y/2-4));
        
            DrawTexturedRect(p, iconSize, "TexClick", white, window);
            DrawTexturedRect(plus, plusSize, "TexPlus", white, window);
            plus = plus + statusIconBuffer;
            DrawTexturedRect(plus, plusSize, "TexMinus", white, window);
            p = p + columnBuffer;

            if (type.hasAbility() || type.hasTargetAbility())
            {
                if (type.hasTargetAbility())
                {
                    if (type.getTargetAbilityType() == ActionTypes::SNIPE)
                    {
                        DrawTexturedRect(p, iconSize, "TexStatusMelee", white, window);
                        DrawString(p + sf::Vector2f(iconSize.x, iconSize.y/2) + iconBuffer, type.getDescription(), white, window);
                        p = p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                    }
                    else if (type.getTargetAbilityType() == ActionTypes::CHILL)
                    {
                        DrawIconAndText(p, iconSize, "TexStatusTap", std::to_string(type.getTargetAbilityAmount()), white, window);
                        p = p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                    }
                }

                GUITools::DrawScriptEffect(type.getAbilityScript(), p, iconSize, halfIconSize, iconBuffer, false, window);
                statusIcon = statusIcon + statusIconBuffer;

                p = statusIcon;

                DrawTexturedRect(p, iconSize, "TexClick", white, window);
                p = p + columnBuffer;

                if (!type.canBlock(true))
                {
                    DrawTexturedRect(p, iconSize, "TexDefenseBig", white, window);
                    p = p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                }

                if (type.usesCharges())
                {
                    DrawIconAndText(p, iconSize, "TexStatusCharge1", std::to_string(type.getChargeUsed()), white, window);
                    p = p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                }

                if (type.getAbilityScript().getDelay() > 0)
                {
                    DrawIconAndText(p, iconSize, "TexStatusDelay", std::to_string(type.getAbilityScript().getDelay()), white, window);
                    p + p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                }

                //GUITools::DrawTexturedRect(plus, plus + plusSize, GUI::TextureMinus, white);

                if (type.getHealthUsed() > 0)
                {
                    DrawIconAndText(p, iconSize, "TexStatusHP", std::to_string(type.getHealthUsed()), white, window);
                    p = p + sf::Vector2f(iconSize.x, 0) + iconBuffer;
                }

                GUITools::DrawScriptCost(type.getAbilityScript(), p, iconSize, halfIconSize, iconBuffer, false, window);
            }
        }    
    }

    void DrawScriptEffect(const Script & script, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window)
    {
        sf::Vector2f halfIconSize(iconSize.x/2, iconSize.y/2);
        sf::Vector2f diffSize((iconSize.x - numberSize.x) / 2, (iconSize.y - numberSize.y) / 2);

        int iconNum = 0;
        sf::Color white2(255, 255, 255, 127);

        sf::Vector2f iconDiff(0, 0);
        
        const ScriptEffect & effect = script.getEffect();

        if (script.isSelfSac())
        {
            DrawTexturedRect(origin + iconDiff, iconSize, "TexStatusSac", sf::Color::White, window); 
            iconDiff = iconDiff  + sf::Vector2f(iconSize.x + buffer.x, 0);
        }

        sf::Vector2f resonateOffset(-8, 0);

        // DRAW MANA RECEIVED
        static std::vector<std::string> ManaStrings = { "TexGold", "TexEnergy", "TexBlue", "TexRed", "TexGreen", "TexAttack" };
        Resources give = effect.getReceive();
        for (size_t m(0); m < Resources::NumTypes; ++m)
        {
            if (drawZeros || (!drawZeros && give.amountOf(m) > 0))
            {
                DrawTexturedRect(origin + iconDiff, iconSize, ManaStrings[m], sf::Color::White, window); 
                DrawTexturedRect(origin + iconDiff + diffSize, numberSize, std::to_string(give.amountOf(m)%21), sf::Color::White, window); 

                if (script.hasResonate())
                {
                    GUITools::DrawTexturedRect(origin + iconDiff + resonateOffset, halfIconSize, script.getResonateEffect().getResonateType().getUIName(), sf::Color::White, window); 
                }

                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }

        // DRAW UNITS CREATED FOR PLAYER
        const std::vector<CreateDescription> & created = effect.getCreate();
        for (size_t i(0); i<created.size(); ++i)
        {
            if (created[i].getOwn())
            {
                CardType createType = created[i].getType();
                DrawIconAndText(origin + iconDiff, iconSize, createType.getUIName(), std::to_string(created[i].getMultiple()), sf::Color::White, window); 
                    
                if (script.hasResonate())
                {
                    DrawTexturedRect(origin + iconDiff + resonateOffset, halfIconSize, script.getResonateEffect().getResonateType().getUIName(), sf::Color::White, window); 
                }

                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }

        // DRAW UNITS CREATED FOR ENEMY
        for (size_t i(0); i<created.size(); ++i)
        {
            if (!created[i].getOwn())
            {
                CardType createType = created[i].getType();
                DrawIconAndText(origin + iconDiff, iconSize, createType.getUIName(), std::to_string(created[i].getMultiple()), sf::Color::White, window); 
                DrawTexturedRect(origin + iconDiff + sf::Vector2f(-5,-5), halfIconSize, "TexNo", sf::Color::White, window);

                if (script.hasResonate())
                {
                    DrawTexturedRect(origin + iconDiff + resonateOffset, halfIconSize, script.getResonateEffect().getResonateType().getUIName(), sf::Color::White, window); 
                }

                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }
    }

    void DrawScriptCost(const Script & script, const sf::Vector2f & origin, const sf::Vector2f & iconSize, const sf::Vector2f & numberSize, const sf::Vector2f & buffer, bool drawZeros, sf::RenderWindow * window)
    {
        sf::Vector2f halfIconSize(iconSize.x/2, iconSize.y/2);
        sf::Vector2f diffSize((iconSize.x - numberSize.x) / 2, (iconSize.y - numberSize.y) / 2);

        int iconNum = 0;

        sf::Vector2f iconDiff(0, 0);
        
        // DRAW MANA COST
        static std::vector<std::string> ManaStrings = { "TexGold", "TexEnergy", "TexBlue", "TexRed", "TexGreen", "TexAttack" };
        Resources cost = script.getManaCost();
        for (size_t m(0); m < Resources::NumTypes; ++m)
        {
            if (drawZeros || (!drawZeros && cost.amountOf(m) > 0))
            {
                DrawTexturedRect(origin + iconDiff, iconSize, ManaStrings[m], sf::Color::White, window); 
                DrawTexturedRect(origin + iconDiff + diffSize, numberSize, std::to_string(cost.amountOf(m)%21), sf::Color::White, window); 
                iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.x, 0);
            }
        }

        // DRAW SAC COST
        const std::vector<SacDescription> & sacCost = script.getSacCost();
        for (size_t i(0); i<sacCost.size(); ++i)
        {
            CardType sacType = sacCost[i].getType();
            DrawIconAndText(origin + iconDiff, iconSize, sacType.getUIName(), std::to_string(sacCost[i].getMultiple()), sf::Color::White, window); 
            iconDiff = iconDiff + sf::Vector2f(iconSize.x + buffer.y, 0);
        }
    }
}
}