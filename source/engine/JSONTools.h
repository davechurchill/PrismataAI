#pragma once

#include "Common.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

#include "Resources.h"
#include "SacDescription.h"
#include "Script.h"
#include "Condition.h"

namespace Prismata
{
namespace JSONTools
{
    template <class T>
    void ReadInt(const char * key, const rapidjson::Value & value, T & dest, bool assertExists = false)
    {
        if (assertExists)
        {
            PRISMATA_ASSERT(value.HasMember(key), "Key not found: %s", key);
        }

        if (value.HasMember(key))
        {
            if (value[key].IsInt())
            {
                dest = (T)value[key].GetInt();
            }
            else if (value[key].IsString())
            {
                dest = (T)atoi(value[key].GetString());
            }
            else
            {
                PRISMATA_ASSERT(false, "%s should be an int/string with int contents", key);
            }
        }
    }

    void ReadIntBool(const char * key, const rapidjson::Value & value, bool & dest, bool assertExists = false);
    void ReadBool(const char * key, const rapidjson::Value & value, bool & dest, bool assertExists = false);
    void ReadDouble(const char * key, const rapidjson::Value & value, double & dest, bool assertExists = false);
    void ReadString(const char * key, const rapidjson::Value & value, std::string & dest, bool assertExists = false);
    void ReadMana(const char * key, const rapidjson::Value & value, Resources & dest, bool assertExists = false);
    void ReadScript(const char * key, const rapidjson::Value & value, Script & dest, bool assertExists = false);
    void ReadScriptEffect(const char * key, const rapidjson::Value & value, ScriptEffect & dest, bool assertExists = false);
    void ReadCondition(const char * key, const rapidjson::Value & value, Condition & dest, bool assertExists = false);
    void ReadSacDescription(const char * key, const rapidjson::Value & value, std::vector<SacDescription> & dest, bool assertExists = false);
}
}
