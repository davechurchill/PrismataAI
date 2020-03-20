#include "BuyLimits.h"

using namespace Prismata;

BuyLimits::BuyLimits()
    : _buyLimits(CardTypes::GetAllCardTypes().size(), 0)
    , _hasLimit(CardTypes::GetAllCardTypes().size(), false)
    , _hasAnyLimits(false)
{

}

BuyLimits::BuyLimits(const rapidjson::Value & val)
    : _buyLimits(CardTypes::GetAllCardTypes().size(), 0)
    , _hasLimit(CardTypes::GetAllCardTypes().size(), false)
    , _hasAnyLimits(false)
{
    PRISMATA_ASSERT(val.IsArray(), "BuyLimits json must be an array");

    for (size_t i(0); i < val.Size(); ++i)
    {
        const rapidjson::Value & limit = val[i];

        PRISMATA_ASSERT(limit.IsArray() && limit.Size() == 2, "BuyLimits array entry must be an array of length 2");
        PRISMATA_ASSERT(limit[0u].IsString() && limit[1u].IsInt(), "Buylimits array entry must be [\"CardName\", Integer]");

        const std::string & cardName = limit[0u].GetString();
        const int cardLimit = limit[1u].GetInt();
        if (CardTypes::CardTypeExists(cardName))
        {
            setLimit(CardTypes::GetCardType(cardName), cardLimit);
            //std::cout << "BuyLimit: " << CardTypes::GetCardType(cardName).getUIName() << " " << cardLimit << "\n";
        }
    }
}

const CardID & BuyLimits::getLimit(const CardType & type) const
{
    return _buyLimits[type.getID()];
}

bool BuyLimits::hasLimit(const CardType & type) const
{
    return _hasLimit[type.getID()];
}

void BuyLimits::setLimit(const CardType & type, const CardID limit)
{
    _hasAnyLimits = true;
    _hasLimit[type.getID()] = true;
    _buyLimits[type.getID()] = limit;
}

bool BuyLimits::hasAnyLimits() const
{
    return _hasAnyLimits;
}

void BuyLimits::addLimits(const BuyLimits & limits)
{
    if (!limits.hasAnyLimits())
    {
        return;
    }

    for (size_t i(0); i < _buyLimits.size(); ++i)
    {
        if (limits._hasLimit[i])
        {
            _hasAnyLimits = true;
            _hasLimit[i] = true;
            _buyLimits[i] = limits._buyLimits[i];
        }
    }
}