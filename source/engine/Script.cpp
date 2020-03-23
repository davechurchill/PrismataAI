#include "Script.h"
#include "CardType.h"

using namespace Prismata;

const Script Script::NullScript = Script();

Script::Script()
    : _delay(0)
    , _selfSac(false)
    , _hasSacCost(false)
    , _hasManaCost(false)
    , _hasResonate(false)
    , _healthUsed(0)
{

}

Script::Script(const rapidjson::Value & value)
    : Script()
{
    if (value.HasMember("delay")) 
    { 
        _delay = (TurnType)value["delay"].GetInt(); 
        
    }

    if (value.HasMember("selfsac"))
    {
        _selfSac = true;
    }
}

bool Script::isSelfSac() const
{
    return _selfSac;
}

const TurnType Script::getDelay() const
{
    return _delay;
}

void Script::setEffect(const ScriptEffect & effect)
{
    _effect = effect;
}

void Script::setHealthUsed(const HealthType health)
{
    _healthUsed = health;
}

void Script::setManaCost(const Resources & cost)
{
    _resourceCost = cost;

    _hasManaCost = !cost.empty();
}

void Script::setSacCost(const std::vector<SacDescription> & sacCost)
{
    _sacCost = sacCost;

    _hasSacCost = !sacCost.empty();
}

bool Script::hasSacCost() const
{
    return _hasSacCost;
}

bool Script::hasManaCost() const
{
    return _hasManaCost;
}

bool Script::hasResonate() const
{
    return _hasResonate;
}

bool Script::hasEffect() const
{
    return _effect.hasEffect() || _resonateEffect.hasEffect() || isSelfSac();
}

void Script::setSelfSac(bool sac)
{
    _selfSac = sac;
}

const std::vector<SacDescription> & Script::getSacCost() const
{
    return _sacCost;
}

const Resources & Script::getManaCost() const
{
    return _resourceCost;
}

const HealthType Script::getHealthUsed() const
{
    return _healthUsed;
}

const ScriptEffect & Script::getEffect() const
{
    return _effect;
}

const ScriptEffect & Script::getResonateEffect() const
{
    return _resonateEffect;
}

void Script::setResonateEffect(const ScriptEffect & effect)
{ 
    _hasResonate = true;
    _resonateEffect = effect;
}

bool Script::operator == (const Script & rhs) const
{
    if (_delay != rhs._delay) return false;
    if (_healthUsed != rhs._healthUsed) return false;
    if (_resourceCost != rhs._resourceCost) return false;
    if (_selfSac != rhs._selfSac) return false;
    if (_hasSacCost != rhs._hasSacCost) return false;
    if (_hasManaCost != rhs._hasManaCost) return false;
    if (_hasResonate != rhs._hasResonate) return false;
    if (!(_effect == rhs._effect)) return false;
    if (!(_resonateEffect == rhs._resonateEffect)) return false;

    if (_sacCost != rhs._sacCost) return false;

    for (size_t i(0); i < _sacCost.size(); ++i)
    {
        if (!(_sacCost[i] == rhs._sacCost[i])) return false;
    }

    return true;
}