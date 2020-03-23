#include "CardTypes.h"
#include "CardTypeData.h"

namespace Prismata
{
namespace CardTypes
{
    std::vector<CardType> allCardTypes;
    std::vector<CardType> baseSetCardTypes;
    std::vector<CardType> dominionCardTypes;

    std::vector< std::vector<CardType> > cardTypeResonatesFrom;
    std::vector< std::vector<CardType> > cardTypeResonatesTo;
    
    void ResetData()
    {
        allCardTypes.clear();
        baseSetCardTypes.clear();
        dominionCardTypes.clear();
        cardTypeResonatesFrom.clear();
        cardTypeResonatesTo.clear();
    }

    void Init()
    {
        ResetData();

        for (CardID i(0); i < CardTypeData::Instance().numCardTypes(); ++i)
        {
            CardType type(i);
            allCardTypes.push_back(type);
            cardTypeResonatesFrom.push_back(std::vector<CardType>());
            cardTypeResonatesTo.push_back(std::vector<CardType>());

            // put the card into the correct set
            if (type.getName().compare("None") != 0)
            {
                (type.isBaseSet() ? baseSetCardTypes : dominionCardTypes).push_back(type);
            }
        }

        // print out information about the card types for debugging
        /*for (CardID i(0); i<CardTypeInfo::numCardTypes(); ++i)
        {
            CardType type(i);
            
            std::cout << i << " " << type.getUIName();

            std::cout << "[";
            for (size_t r(0); r < type.getResonateFromIDs().size(); ++r)
            {
                const CardID rFromID = type.getResonateFromIDs()[r];
                std::cout << CardTypes::GetAllCardTypes()[rFromID].getUIName() << ", ";
            }

            std::cout << "], [";
            for (size_t r(0); r < type.getResonateToIDs().size(); ++r)
            {
                const CardID rToID = type.getResonateToIDs()[r];
                std::cout << CardTypes::GetAllCardTypes()[rToID].getUIName() << ", ";
            }
            std::cout << "]\n";
        }*/

        std::cout << "Base     set has " << baseSetCardTypes.size() << " cards\n";
        std::cout << "Dominion set has " << dominionCardTypes.size() << " cards\n";
    }

    bool IsBaseSet(const CardType type)
    {
        return std::find(baseSetCardTypes.begin(), baseSetCardTypes.end(), type) != baseSetCardTypes.end();
    }

    CardType GetCardType(const std::string & name)
    {
        for (size_t i(0); i<allCardTypes.size(); ++i)
        {
            if (allCardTypes[i].getName().compare(name) == 0 || allCardTypes[i].getUIName().compare(name) == 0)
            {
                return allCardTypes[i];
            }
        }

        PRISMATA_ASSERT(false, "CardType::getCardType() error: Card name not found: %s", name.c_str());

        return allCardTypes[0];
    }

    bool CardTypeExists(const std::string & name)
    {
        for (size_t i(0); i<allCardTypes.size(); ++i)
        {
            if (allCardTypes[i].getName().compare(name) == 0 || allCardTypes[i].getUIName().compare(name) == 0)
            {
                return true;
            }
        }

        return false;
    }

    const CardType GetCardType(const CardID cardID)
    {
        return allCardTypes[cardID];
    }

    const std::vector<CardType> & GetAllCardTypes()
    {
        return allCardTypes;
    }

    const std::vector<CardType> & GetBaseSetCardTypes()
    {
        return baseSetCardTypes;
    }

    const std::vector<CardType> & GetDominionCardTypes()
    {
        return dominionCardTypes;
    }
}
}