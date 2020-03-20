#pragma once

#include "Common.h"
#include "Player.h"
#include "Game.h"
#include "GameState.h"
#include "CardType.h"
#include "Resources.h"
#include "CardTypeInfo.h"
#include "FileUtils.h"
#include "Player.h"
#include <string>

namespace Prismata
{
    extern std::string AIConfigFile;
    extern bool PRISMATA_INITIALIZED;

    // initializes all card info from cardLibrary.jso file
    void InitFromCardLibrary(const std::string & jsonGameStateCardDataFile);

    void InitFromMergedDeckJSON(const rapidjson::Value & mergedDeckArray);
    
    void ResetData();
}