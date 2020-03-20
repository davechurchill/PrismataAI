#pragma once

#include <map>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

namespace Prismata
{
class Assets
{
    sf::Texture m_blankTexture;

    std::map<std::string, sf::Texture>      m_textureMap;
    std::map<std::string, sf::Font>         m_fontMap;
    std::map<std::string, sf::SoundBuffer>  m_soundBufferMap;
    std::map<std::string, sf::Sound>        m_soundMap;

    Assets();

public:
    
    static Assets& Instance();

    void addTexture(const std::string& textureName, const std::string& path, bool smooth = true);
    void addFont(const std::string& fontName, const std::string& path);
    void addSound(const std::string& fontName, const std::string& path);

    const sf::Texture & getTexture(const std::string & textureName) const;
    const sf::Font &    getFont(const std::string & fontName) const;
    sf::Sound &         getSound(const std::string& soundName);
};
}