#include "Prismata.h"
#include "CardTypeData.h"

namespace Prismata
{
    bool PRISMATA_INITIALIZED = false;
    std::string AIConfigFile = "No Filename";
    
    // this initialization function should only ever be called if compiling to C++ native for testing
    void InitFromCardLibrary(const std::string & jsonGameStateCardDataFile)
    {
        AIConfigFile = jsonGameStateCardDataFile;
        ResetData();

        // Initialize CardType data
        CardTypeData::Instance().InitFromCardLibraryFile(jsonGameStateCardDataFile);
        
        CardTypes::Init();
            
        PRISMATA_INITIALIZED = true;
    }

    void InitFromMergedDeckJSON(const rapidjson::Value & mergedDeckArray)
    {
        ResetData();

        PRISMATA_ASSERT(mergedDeckArray.IsArray(), "mergedDeck is not a JSON Array");
        CardTypeData::Instance().InitFromMergedDeckJSON(mergedDeckArray);

        // Initialize CardTypes with the base and dominion arrays
        CardTypes::Init();
            
        PRISMATA_INITIALIZED = true;
    }

    void ResetData()
    {
        CardTypes::ResetData();
    }
}