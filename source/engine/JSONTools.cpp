#include "JSONTools.h"


namespace Prismata
{
namespace JSONTools
{
    void ReadIntBool(const char * key, const rapidjson::Value & value, bool & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsInt(), "%s should be an int", key);
            dest = value[key].GetInt() != 0;
        }
    }

    void ReadBool(const char * key, const rapidjson::Value & value, bool & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsBool(), "%s should be a bool", key);
            dest = value[key].GetBool();
        }
    }

    void ReadDouble(const char * key, const rapidjson::Value & value, double & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsDouble(), "%s should be a double", key);
            dest = value[key].GetDouble();
        }
    }

    void ReadString(const char * key, const rapidjson::Value & value, std::string & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsString(), "%s should be a string", key);
            dest = value[key].GetString();
        }
    }

    void ReadMana(const char * key, const rapidjson::Value & value, Resources & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsString() || value[key].IsInt(), "%s should be a string or an int", key);
            dest = Prismata::Resources(value[key]);
        }
    }

    void ReadScriptEffect(const char * key, const rapidjson::Value & value, ScriptEffect & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsObject(), "%s should be an Object", key);
            dest = Prismata::ScriptEffect(value[key]);
        }
    }

    void ReadScript(const char * key, const rapidjson::Value & value, Script & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsObject(), "%s should be an Object", key);
            dest = Prismata::Script(value[key]);
        }
    }

    void ReadCondition(const char * key, const rapidjson::Value & value, Condition & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsObject(), "%s should be an Object", key);
            dest = Prismata::Condition(value[key]);
        }
    }

    void ReadSacDescription(const char * key, const rapidjson::Value & value, std::vector<SacDescription> & dest, bool assertExists)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            PRISMATA_ASSERT(value[key].IsArray(), "%s should be an array", key);
        
            for (size_t i = 0; i < value[key].Size(); i++) 
            { 
                dest.push_back(SacDescription(value[key][i])); 
            }
        }
    }
}
}

