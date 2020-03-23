#include "ScriptEffect.h"
#include "CardType.h"

using namespace Prismata;

ScriptEffect::ScriptEffect()
    : m_hasEffect(false)
    , m_resonateTypeID(0)
{

}

ScriptEffect::ScriptEffect(const rapidjson::Value & value)
    : ScriptEffect()
{
    if (value.HasMember("receive")) 
    { 
        m_receive = Resources(value["receive"]); 
        m_hasEffect = true;
    }
                        
    if (value.HasMember("opponentReceive"))
    { 
        m_give = Resources(value["opponentReceive"]); 
        m_hasEffect = true;
    }

    if (value.HasMember("create")) 
    {
        int loopmax = value["create"].Size();
        
        for (int i = 0; i < loopmax; i++) 
        { 
            m_create.push_back(CreateDescription(value["create"][i])); 
        }

        m_hasEffect = true;
    }

    if (value.HasMember("destroy")) 
    {
        int loopmax = value["destroy"].Size();
        
        for (int i = 0; i < loopmax; i++) 
        { 
            m_destroy.push_back(DestroyDescription(value["destroy"][i])); 
        }

        m_hasEffect = true;
    }
}

bool ScriptEffect::hasEffect() const
{
    return m_hasEffect;
}

const Resources & ScriptEffect::getGive() const
{
    return m_give;
}

const Resources & ScriptEffect::getReceive() const
{
    return m_receive;
}

const std::vector<CreateDescription> & ScriptEffect::getCreate() const
{
    return m_create;
}

const std::vector<DestroyDescription> & ScriptEffect::getDestroy() const
{
    return m_destroy;
}

const int ScriptEffect::getAttackValue() const
{
    return m_receive.amountOf(Resources::Attack);
}

void ScriptEffect::addCreateEffect(const CreateDescription & create)
{
    m_create.push_back(create);
    m_hasEffect = true;
}

const CardID ScriptEffect::getResonateTypeID() const
{
    if (m_resonateTypeID == 0)
    {
        m_resonateTypeID = CardTypes::GetCardType(m_resonateCardName).getID();
    }

    return m_resonateTypeID;
}

const std::string & ScriptEffect::getResonateTypeName() const
{
    return m_resonateCardName;
}

const CardType ScriptEffect::getResonateType() const
{
    return CardTypes::GetAllCardTypes()[getResonateTypeID()];
}

ScriptEffect ScriptEffect::ResonateEffect(const std::string & cardName, const Resources & receive)
{
    ScriptEffect effect;

    effect.m_hasEffect = true;
    effect.m_resonateCardName = cardName;
    effect.m_receive = receive;

    return effect;
}

bool ScriptEffect::operator == (const ScriptEffect & rhs) const
{
    if (m_receive != rhs.m_receive) return false;
    if (m_give != rhs.m_give) return false;
    if (m_resonateCardName != rhs.m_resonateCardName) return false;
    if (m_hasEffect != rhs.m_hasEffect) return false;

    if (m_destroy.size() != rhs.m_destroy.size()) return false;
    if (m_create.size() != rhs.m_create.size()) return false;

    for (size_t i(0); i < m_destroy.size(); ++i)
    {
        if (!(m_destroy[i] == rhs.m_destroy[i])) return false;
    }

    for (size_t i(0); i < m_create.size(); ++i)
    {
        if (!(m_create[i] == rhs.m_create[i])) return false;
    }

    return true;
}