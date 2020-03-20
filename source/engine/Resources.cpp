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

Resources::Resources(const ResourceType & p, const ResourceType & h, const ResourceType & b, const ResourceType & c, const ResourceType & g, const ResourceType & a)
    : Resources()
{
    _pool[Resources::Gold] = p;
    _pool[Resources::Energy] = h;
    _pool[Resources::Blue] = b;
    _pool[Resources::Red] = c;
    _pool[Resources::Green] = g;
    _pool[Resources::Attack] = a;
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
    return _pool[Resources::Gold] == 0 && _pool[Resources::Energy] == 0 && _pool[Resources::Blue] == 0 && _pool[Resources::Red] == 0 && _pool[Resources::Green] == 0 && _pool[Resources::Attack] == 0;
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
    _pool[0] *= val;
    _pool[1] *= val;
    _pool[2] *= val;
    _pool[3] *= val;
    _pool[4] *= val;
    _pool[5] *= val;
}

void Resources::add(const Resources & m)
{
    _pool[0] += m._pool[0];
    _pool[1] += m._pool[1];
    _pool[2] += m._pool[2];
    _pool[3] += m._pool[3];
    _pool[4] += m._pool[4];
    _pool[5] += m._pool[5];
}

void Resources::subtract(const Resources & m)
{
    PRISMATA_ASSERT(has(m), "Mana::subtract() error: Don't have enough to subtract!");

    _pool[0] -= m._pool[0];
    _pool[1] -= m._pool[1];
    _pool[2] -= m._pool[2];
    _pool[3] -= m._pool[3];
    _pool[4] -= m._pool[4];
    _pool[5] -= m._pool[5];
}

void Resources::set(const Resources & m)
{
    _pool[0] = m._pool[0];
    _pool[1] = m._pool[1];
    _pool[2] = m._pool[2];
    _pool[3] = m._pool[3];
    _pool[4] = m._pool[4];
    _pool[5] = m._pool[5];
}

const ResourceType & Resources::amountOf(const size_t & resourceType) const
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::amountOf() error: Mana type index not known: %d", resourceType);

    return _pool[resourceType];
}

void Resources::add(const size_t & resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::add() error: Mana type index not known: %d", resourceType);

    _pool[resourceType] += val;
}

void Resources::subtract(const size_t & resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::subtract() error: Mana type index not known: %d", resourceType);
    PRISMATA_ASSERT(_pool[resourceType] >= val, "Mana::subtract() error: Did not have enough of given type: %d", resourceType);

    _pool[resourceType] -= val;
}

void Resources::set(const size_t & resourceType, const ResourceType val)
{
    PRISMATA_ASSERT(resourceType < Resources::NumTypes, "Mana::set() error: Mana type index not known: %d", resourceType);
    _pool[resourceType] = val;
}

bool Resources::has(const Resources & m) const
{
    if (_pool[0] < m._pool[0]) return false;
    if (_pool[1] < m._pool[1]) return false;
    if (_pool[2] < m._pool[2]) return false;
    if (_pool[3] < m._pool[3]) return false;
    if (_pool[4] < m._pool[4]) return false;
    if (_pool[5] < m._pool[5]) return false;
    
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
