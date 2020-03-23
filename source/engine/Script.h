#pragma once

#include "Common.h"
#include "Resources.h"
#include "CreateDescription.h"
#include "ScriptEffect.h"
#include "SacDescription.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{
namespace ScriptTypes
{
enum { BuyScript, AbilityScript, BeginTurnScript, Size };
}

class Script
{
    TurnType                    _delay;
    HealthType                  _healthUsed;
    Resources                   _resourceCost;
    std::vector<SacDescription> _sacCost;
    ScriptEffect                _effect;
    ScriptEffect                _resonateEffect;
    bool                        _selfSac;
    bool                        _hasSacCost;
    bool                        _hasManaCost;
    bool                        _hasResonate;


public:

    static const Script NullScript;

    Script();
    Script(const rapidjson::Value & value);

    bool hasEffect()      const;
    bool hasManaCost()    const;
    bool hasSacCost()     const;
    bool isSelfSac()      const;
    bool hasResonate()    const;
    const TurnType getDelay()                         const;
    const ScriptEffect & getEffect()                    const;
    const ScriptEffect & getResonateEffect()            const;
    const std::vector<SacDescription> & getSacCost()    const;
    const Resources & getManaCost()                     const;
    const HealthType getHealthUsed()                  const;

    bool operator == (const Script & rhs) const;

    void setEffect(const ScriptEffect & effect);
    void setResonateEffect(const ScriptEffect & effect);
    void setHealthUsed(const HealthType health);
    void setManaCost(const Resources & cost);
    void setSacCost(const std::vector<SacDescription> & sacCost);
    void setSelfSac(bool sac);
};

}