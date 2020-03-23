#pragma once

#include "CardTypeInfo.h"

namespace Prismata
{
 
class CardTypeData
{
    std::vector<CardTypeInfo> m_allCardTypeInfo;
    
    CardTypeData();

public:

    static CardTypeData & Instance();
    
    const CardTypeInfo & getCardTypeInfo(const CardID id);
    const CardTypeInfo & GetCardTypeInfoByName(const std::string & name);
    
    void ProcessPostInit();
    void ResetData();
    void InitFromCardLibraryFile(const std::string & jsonGameStateCardData);
    void InitFromMergedDeckJSON(const rapidjson::Value & mergedDeck);
    void printCardTypeVariableNames();
    std::string getVariableName(const std::string & str);
    size_t numCardTypes();
};

}