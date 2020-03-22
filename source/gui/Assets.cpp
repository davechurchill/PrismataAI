#include "Assets.h"
#include <cassert>
#include <fstream>
#include <iostream>

using namespace Prismata;

Assets::Assets()
{

}

Assets& Assets::Instance()
{
    static Assets instance;
    return instance;
}

void Assets::addTexture(const std::string & textureName, const std::string & path, bool smooth)
{
    m_textureMap[textureName] = sf::Texture();

    if (!m_textureMap[textureName].loadFromFile(path))
    {
        std::cerr << "Could not load texture file: " << path << std::endl;
        // m_textureMap.erase(textureName);
    }
    else
    {
        m_textureMap[textureName].setSmooth(smooth);
        std::cout << "Loaded Texture: " << path << std::endl;
    }
}

const sf::Texture & Assets::getTexture(const std::string & textureName) const
{
    return m_textureMap.at(textureName);
}

void Assets::addFont(const std::string & fontName, const std::string & path)
{
    m_fontMap[fontName] = sf::Font();
    if (!m_fontMap[fontName].loadFromFile(path))
    {
        std::cerr << "Could not load font file: " << path << std::endl;
        m_fontMap.erase(fontName);
    }
    else
    {
        std::cout << "Loaded Font:    " << path << std::endl;
    }
}

const sf::Font & Assets::getFont(const std::string & fontName) const
{
    assert(m_fontMap.find(fontName) != m_fontMap.end());
    return m_fontMap.at(fontName);
}

void Assets::addSound(const std::string& soundName, const std::string& path)
{
    m_soundBufferMap[soundName] = sf::SoundBuffer();
    if (!m_soundBufferMap[soundName].loadFromFile(path))
    {
        std::cerr << "Could not load sound file: " << path << std::endl;
        m_soundBufferMap.erase(soundName);
    }
    else
    {
        std::cout << "Loaded Sound:    " << path << std::endl;
        m_soundMap[soundName] = sf::Sound(m_soundBufferMap[soundName]);
        m_soundMap[soundName].setVolume(25);
    }
}

sf::Sound & Assets::getSound(const std::string& soundName) 
{
    assert(m_soundMap.find(soundName) != m_soundMap.end());
    return m_soundMap.at(soundName);
}