#include "Resources.h"

using namespace Prismata;

Resources::Resources()
{

}

Resources::Resources(const rapidjson::Value & value)
    : Resources()
{
    if (value.IsInt())
    {
        set(Resources::Gold, value.GetInt());
    }
    else if (value.IsString())
    {
        setFromString(value.GetString());
    }
}

Resources::Resources(const std::string & resourceString)
    : Resources()
{
    setFromString(resourceString);
}

Resources::Resources(const ResourceType p, const ResourceType h, const ResourceType b, const ResourceType c, const ResourceType g, const ResourceType a)
    : Resources()
{
    m_pool[Resources::Gold] = p;
    m_pool[Resources::Energy] = h;
    m_pool[Resources::Blue] = b;
    m_pool[Resources::Red] = c;
    m_pool[Resources::Green] = g;
    m_pool[Resources::Attack] = a;
}

bool Resources::operator == (const Resources & rhs) const
{
    for (size_t i(0); i<Resources::NumTypes; ++i)
    {
        if (amountOf(i) != rhs.amountOf(i))
        {
            return false;
        }
    }

    return true;
}

bool Resources::empty() const
{
    return m_pool[Resources::Gold] == 0 && m_pool[Resources::Energy] == 0 && m_pool[Resources::Blue] == 0 && m_pool[Resources::Red] == 0 && m_pool[Resources::Green] == 0 && m_pool[Resources::Attack] == 0;
}

bool Resources::operator != (const Resources & rhs) const
{
    return !(*this == rhs);
}

void Resources::setFromString(const std::string & resourceString)
{
    // parse the integer prefix of the resource string
    char intString[16];
    size_t i = 0;

    while (true)
    {
        // if it's a digit
        if (resourceString[i] >= '0' && resourceString[i] <= '9')
        {
            // add it to the intString
            intString[i] = resourceString[i];
            i++;
        }
        // otherwise it's not a digit
        else
        {
            intString[i] = '\0';

            if (i > 0)
            {
                // parse the intString and break out
                set(Resources::Gold, atoi(intString));
            }

            break;
        }
    }

    // parse the character suffix of the resource string
    while (i < resourceString.size()) 
    {
        if (     resourceString[i] == Resources::GetChar(Energy))   { add(Resources::Energy, 1);   }
        else if (resourceString[i] == Resources::GetChar(Blue))     { add(Resources::Blue, 1);     }
        else if (resourceString[i] == Resources::GetChar(Red))      { add(Resources::Red, 1);      }
        else if (resourceString[i] == Resources::GetChar(Green))    { add(Resources::Green, 1);    }
        else if (resourceString[i] == Resources::GetChar(Attack))   { add(Resources::Attack, 1);   }
        
        i++;
    }
}

void Resources::multiply(const ResourceType val)
{
    m_pool[0] *= val;
    m_pool[1] *= val;
    m_pool[2] *= val;
    m_pool[3] *= val;
    m_pool[4] *= val;
    m_pool[5] *= val;
}

void Resources::add(const Resources & m)
{
    m_pool[0] += m.m_pool[0];
    m_pool[1] += m.m_pool[1];
    m_pool[2] += m.m_pool[2];
    m_pool[3] += m.m_pool[3];
    m_pool[4] += m.m_pool[4];
    m_pool[5] += m.m_pool[5];
}

void Resources::subtract(const Resources & m)
{
    PRISMATA_ASSERT(has(m), "Mana::subtract() error: Don't have enough to subtract!");

    m_pool[0] -= m.m_pool[0];
    m_pool[1] -= m.m_pool[1];
    m_pool[2] -= m.m_pool[2];
    m_pool[3] -= m.m_pool[3];
    m_pool[4] -= m.m_pool[4];
    m_pool[5] -= m.m_pool[5];
}

void Resources::set(const Resources & m)
{
    m_pool[0] = m.m_pool[0];
    m_pool[1] = m.m_pool[1];
    m_pool[2] = m.m_pool[2];
    m_pool[3] = m.m_pool[3];
    m_pool[4] = m.m_pool[4];
    m_pool[5] = m.m_pool[5];
}

const ResourceType & Resources::amountOf(const size_t & resourceType) const
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::amountOf() error: Mana type index not known: %d", resourceType);

    return m_pool[resourceType];
}

void Resources::add(const size_t resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::add() error: Mana type index not known: %d", resourceType);

    m_pool[resourceType] += val;
}

void Resources::subtract(const size_t resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::subtract() error: Mana type index not known: %d", resourceType);
    PRISMATA_ASSERT(m_pool[resourceType] >= val, "Mana::subtract() error: Did not have enough of given type: %d", resourceType);

    m_pool[resourceType] -= val;
}

void Resources::set(const size_t resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::set() error: Mana type index not known: %d", resourceType);
    m_pool[resourceType] = val;
}

bool Resources::has(const Resources & m) const
{
    if (m_pool[0] < m.m_pool[0]) return false;
    if (m_pool[1] < m.m_pool[1]) return false;
    if (m_pool[2] < m.m_pool[2]) return false;
    if (m_pool[3] < m.m_pool[3]) return false;
    if (m_pool[4] < m.m_pool[4]) return false;
    if (m_pool[5] < m.m_pool[5]) return false;
    
    return true;
}

const std::string Resources::getString() const
{
    std::stringstream ss;

    if (amountOf(Resources::Gold) > 0)
    {
        ss << amountOf(Resources::Gold);
    }

    for (size_t i(1); i<Resources::NumTypes; ++i)
    {
        for (int n(0); n<amountOf(i); ++n)
        {
            ss << Resources::GetChar(i);        
        }
    }

    return ss.str();
}

const std::string Resources::getIntString() const
{
    std::stringstream ss;

    for (size_t i(0); i<Resources::NumTypes; ++i)
    {
        if (amountOf(i) > 0)
        {
            ss << (int)amountOf(i) << Resources::GetChar(i) << " ";
        }
    }

    return ss.str();
}


char Resources::GetChar(size_t resourceIndex)
{
    static const char characters[7] = {'P', 'H', 'B', 'C', 'G', 'A', 'S'};
    return characters[resourceIndex];
}

char Resources::GetCharReal(size_t resourceIndex)
{
    static const char characters[7] = {'$', 'E', 'B', 'R', 'G', 'A', 'S'};
    return characters[resourceIndex];
}
