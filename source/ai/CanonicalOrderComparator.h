#pragma once

#include "Common.h"
#include "CardType.h"
#include "Card.h"
#include "GameState.h"

namespace Prismata
{

class CanonicalOrderComparator 
{
    const GameState & _state;

    size_t getCanonicalIndex(const Card & card) const;

public:

    CanonicalOrderComparator(const GameState & state);

    bool operator() (const CardID & cardID1, const CardID & cardID2) const;
};
}