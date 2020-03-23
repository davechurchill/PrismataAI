#pragma once

#include "Common.h"
#include "Script.h"
#include "Resources.h"
#include "CardType.h"

namespace Prismata
{
namespace CardTypes
{
    void ResetData();
    void Init();
    const std::vector<CardType> & GetAllCardTypes();
    const std::vector<CardType> & GetBaseSetCardTypes();
    const std::vector<CardType> & GetDominionCardTypes();
    CardType GetCardType(const std::string & name);
    bool CardTypeExists(const std::string & name);
    bool IsBaseSet(const CardType type);
    extern const CardType None;
}
}