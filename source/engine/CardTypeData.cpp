#include "CardTypeData.h"
#include "CardTypes.h"
#include "FileUtils.h"

using namespace Prismata;

CardTypeData::CardTypeData()
{

}

CardTypeData & CardTypeData::Instance()
{
    static CardTypeData params;
    return params;
}

void CardTypeData::ProcessPostInit()
{
    for (size_t type(0); type < m_allCardTypeInfo.size(); ++type)
    {
        CardTypeInfo & data = m_allCardTypeInfo[type];
        data.attackGivenToEnemy = 0;

        // calculate how much attack buying this card gives to the enemy
        data.attackGivenToEnemy += data.buyScript.getEffect().getGive().amountOf(Resources::Attack);

        const std::vector<CreateDescription> & created = data.buyScript.getEffect().getCreate();
        for (size_t c(0); c < created.size(); ++c)
        {
            // if we give this to the enemy
            if (!created[c].getOwn())
            {
                const CardTypeInfo CardTypeInfo = GetCardTypeInfoByName(created[c].getCardName());

                data.attackGivenToEnemy += CardTypeInfo.attack * (HealthType)created[c].getMultiple();
            }
        }
        
        // process resonate to and resonate from
        if (data.beginOwnTurnScript.hasResonate())
        {
            CardID resonateFromID = GetCardTypeInfoByName(data.beginOwnTurnScript.getResonateEffect().getResonateTypeName()).typeID;
            data.resonatesFromIDs.push_back(resonateFromID);
            m_allCardTypeInfo[resonateFromID].resonatesToIDs.push_back(type);
        }
    }
}
const CardTypeInfo & CardTypeData::getCardTypeInfo(const CardID id)
{
    PRISMATA_ASSERT(id < m_allCardTypeInfo.size(), "Card ID not known: %d", id);

    return m_allCardTypeInfo[id];
}

const CardTypeInfo & CardTypeData::GetCardTypeInfoByName(const std::string & name)
{
    for (CardID c(0); c<m_allCardTypeInfo.size(); ++c)
    {
        if (m_allCardTypeInfo[c].cardName.compare(name) == 0)
        {
            return m_allCardTypeInfo[c];
        }
    }

    return m_allCardTypeInfo[0];
}

std::string CardTypeData::getVariableName(const std::string & str)
{
    char temp[256];
    size_t ind = 0;
    for (size_t i(0); i < str.size(); ++i)
    {
        if (str[i] == ' ')
        {
            if (ind > 0 && temp[ind - 1] != '_')
            {
                temp[ind++] = '_';
            }
        }
        else if (str[i] == '-')
        {

        }
        else
        {
            temp[ind++] = str[i];
        }
    }

    temp[ind] = '\0';
    return std::string(temp);
}

size_t CardTypeData::numCardTypes()
{
    return m_allCardTypeInfo.size();
}

void CardTypeData::ResetData()
{
    m_allCardTypeInfo.clear();
}

void CardTypeData::InitFromMergedDeckJSON(const rapidjson::Value & mergedDeck)
{
    Instance() = CardTypeData();

    m_allCardTypeInfo.push_back(CardTypeInfo());
    m_allCardTypeInfo.push_back(CardTypeInfo());

    PRISMATA_ASSERT(mergedDeck.IsArray(), "Input 'mergedDeck' JSON Value is not an Array");
    
    for (rapidjson::SizeType i=0; i<mergedDeck.Size(); ++i)
    {
        PRISMATA_ASSERT(mergedDeck[i].HasMember("name"), "Deck element has no name field");

        const std::string &         name = mergedDeck[i]["name"].GetString();
        const rapidjson::Value &    val  = mergedDeck[i];

        m_allCardTypeInfo.push_back(CardTypeInfo(m_allCardTypeInfo.size(), name, val));
    }

    ProcessPostInit();

}

void CardTypeData::InitFromCardLibraryFile(const std::string & jsonGameStateCardData)
{
    Instance() = CardTypeData();

    m_allCardTypeInfo.push_back(CardTypeInfo());
    m_allCardTypeInfo.push_back(CardTypeInfo());

    rapidjson::Document document;
    
    const std::string dominionNames[] = 
    {
        "Vivid Drone", "Bond", "Centrifuge", "Doomed Drone", "Battalion", "Panther", "Flame Warrior", "Flame Kin", "Machine", "Cyclic Attacker",
        "Fragilewall", "Fabricator", "Distractorod", "Ion Cannon", "Meteor Shower", "Wild Drone",
        "Doomed Mech", "Conscription", "Pixie", "Blade", "Supertreant", "Golem", "Defense Grid", "Drake", "Militia", "Doomwall", "Polywall", "Nether Warrior", "Flame Assassin", "Hate Reactor", "Adrenaline Reactor", "Furion",
        "Fickle Marine", "Screech Blast", "House", "Trickster", "Gnoll", "Angelic", "Annihilator", "Nightmare Cannon", "Jester", "Doomed Infantry", "Neo Overlord",
        "Antima Comet", "BFD", "Stone Guardian", "Tesla Coil", "Factory",
        "Piranha Academy", "Statue", "Minimarshal", "Demolition Mech", "Grenadier",
        "Ephemeron", "Sentinel", "Gasplant", "Rocket Artillery", "HPMan", "Pixieflower",
        "NeoContraption", "Unholy Barrier", "Electrophore", "Sound Barrier", "Uberdefcell", "Giga Cannon", "Psychosis Cannon", "Tank", "Marauder", "Deconstructible Tower", "Cowardly Marine", "Hotel", "Disruption Kit", "Viletrope", "Roshan", "Rukh",
        "Ebb Turbine", "Beam of Wincing", "Volatile Blast", "Arsonist",
        "Butter on Blood", "Husk",
        "Nitrocybe", "Corpus", "Vai Mauronax", "Xaetron", "Vai Mauronax", "Corpus",
        "Borehole Patroller", "Thorium Dynamo", "Tantalum Ray",
        "Psychosis Cannon", "Blood Phage", "Galvani Drone", "Thunderhead",
        "Savior", "Lancetooth", "Ferritin Sac"
    };

    bool parsingFailed = document.Parse(FileUtils::ReadFile(jsonGameStateCardData).c_str()).HasParseError();

    PRISMATA_ASSERT(!parsingFailed, "Couldn't parse card library file");

    for (rapidjson::Value::ConstMemberIterator itr = document.MemberBegin(); itr != document.MemberEnd(); ++itr)
    {
        const std::string &         name = itr->name.GetString();
        const rapidjson::Value &    val  = itr->value;

        bool isBaseSet = val.HasMember("baseSet") && val["baseSet"].IsInt() && (val["baseSet"].GetInt() == 1);
        bool isDominionSet = std::find(std::begin(dominionNames), std::end(dominionNames), name) != std::end(dominionNames);
               
        if (isBaseSet || isDominionSet)
        {
            m_allCardTypeInfo.push_back(CardTypeInfo(m_allCardTypeInfo.size(), name, val));
        }
    }

    ProcessPostInit();
}

void CardTypeData::printCardTypeVariableNames()
{
    for (size_t i(0); i < m_allCardTypeInfo.size(); ++i)
    {
        const CardTypeInfo & typeData = m_allCardTypeInfo[i];
        std::cout << "CardType " << getVariableName(typeData.cardName) << "(" << typeData.typeID << ");" << std::endl;
        
    }

    for (size_t i(0); i < m_allCardTypeInfo.size(); ++i)
    {
        const CardTypeInfo & typeData = m_allCardTypeInfo[i];
        std::cout << "extern CardType " << getVariableName(typeData.cardName) << ";" << std::endl;
    }
}
