#pragma once

#include "Common.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include <array>

namespace Prismata
{
 
class Resources
{
    std::array<ResourceType, 6> m_pool = {};

    void setFromString(const std::string & resourceString);

public:
    
    // This enumeration defines specific integers for each resource type so we can refer to them by variable
    enum { Gold = 0, Energy = 1, Blue = 2, Red = 3, Green = 4, Attack = 5, NumTypes = 6, Sac = 255};

    Resources();
    Resources(const rapidjson::Value & value);
    Resources(const std::string & resourceString);
    Resources(const ResourceType p, const ResourceType h, const ResourceType b, const ResourceType c, const ResourceType g, const ResourceType a);
    
    bool operator == (const Resources & rhs) const;
    bool operator != (const Resources & rhs) const;

    const ResourceType & amountOf(const size_t & resourceType) const;
    bool has(const Resources & m) const;
    const std::string getString() const;
    const std::string getIntString() const;

    void add(const size_t resourceType, const ResourceType val);
    void add(const Resources & m);
    void subtract(const size_t resourceType, const ResourceType val);
    void subtract(const Resources & m);
    void set(const size_t resourceType, const ResourceType val);
    void set(const Resources & m);
    void multiply(const ResourceType val);

    bool empty() const;

    static char GetChar(size_t resourceIndex);
    static char GetCharReal(size_t resourceIndex);
};

}