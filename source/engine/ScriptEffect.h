#pragma once

#include "Common.h"
#include "Resources.h"
#include "CreateDescription.h"
#include "DestroyDescription.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

namespace Prismata
{

class ScriptEffect
{
    std::vector<DestroyDescription> m_destroy;
    std::vector<CreateDescription>  m_create;
    Resources                       m_receive;
    Resources                       m_give;
    std::string                     m_resonateCardName;
    mutable CardID                  m_resonateTypeID;
    bool                            m_hasEffect;

public:

    ScriptEffect();
    ScriptEffect(const rapidjson::Value & value);

    void addCreateEffect(const CreateDescription & create);
    
    bool operator == (const ScriptEffect & rhs)       const;
    const int getAttackValue()                              const;
    bool hasEffect()                                  const;
    const CardID getResonateTypeID()                      const;
    const CardType getResonateType()                      const;
    const std::string & getResonateTypeName()               const;
    const Resources & getGive()                             const;
    const Resources & getReceive()                          const;
    const std::vector<CreateDescription> & getCreate()      const;
    const std::vector<DestroyDescription> & getDestroy()    const;

    static ScriptEffect ResonateEffect(const std::string & cardName, const Resources & receive);
};

}