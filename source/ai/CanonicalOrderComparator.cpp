#include "CanonicalOrderComparator.h"

using namespace Prismata;

CanonicalOrderComparator::CanonicalOrderComparator(const GameState & state)
    : _state(state)
{
}


// canonical move ordering: H > G > R > B > A > $

const std::vector<size_t> CanonicalOrder = { Resources::Energy, Resources::Green, Resources::Red, Resources::Blue, Resources::Attack, Resources::Gold };

bool CanonicalOrderComparator::operator() (const CardID cardID1, const CardID cardID2) const
{
    const Card & card1 = _state.getCardByID(cardID1);
    const Card & card2 = _state.getCardByID(cardID2);

    return getCanonicalIndex(card1) < getCanonicalIndex(card2);
}

size_t CanonicalOrderComparator::getCanonicalIndex(const Card & card) const
{
    const Resources & resource = card.getType().getAbilityScript().getEffect().getReceive();

    if (resource.amountOf(Resources::Energy) > 0)  { return 0; }
    if (resource.amountOf(Resources::Green) > 0)   { return 1; }
    if (resource.amountOf(Resources::Red) > 0)     { return 2; }
    if (resource.amountOf(Resources::Blue) > 0)    { return 3; }
    if (resource.amountOf(Resources::Attack) > 0)  { return 4; }
    if (resource.amountOf(Resources::Gold) > 0) { return 5; }

    return 6;
}