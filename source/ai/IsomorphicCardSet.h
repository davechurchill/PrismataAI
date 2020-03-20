#pragma once

#include "Common.h"
#include "GameState.h"

namespace Prismata
{

class IsomorphicCardSet
{
    std::vector<CardID> _cardIDs;
    size_t              _numUsed;

public:

    IsomorphicCardSet();
    
    void add(const CardID & cardID);
    void incUsed();
    void decUsed();

    bool isIsomorphic(const GameState & state, const CardID & otherCardID) const;
    const std::vector<CardID> & getCardIDs() const;
    const size_t size() const;
    const size_t & numUsed() const;
    const Card & getCurrentCard(const GameState & state) const;
    const CardID & getCurrentCardID() const;
    bool allUsed() const;
};
 
class IsomorphicBlockerComparator 
{
    const GameState & _state;

public:

    IsomorphicBlockerComparator(const GameState & state)
        : _state(state)
    {
    }

    bool operator() (const IsomorphicCardSet & ics1, const IsomorphicCardSet & ics2) const
    {
        const Card & card1 = ics1.getCurrentCard(_state);
        const Card & card2 = ics2.getCurrentCard(_state);

        return card1.currentHealth() > card2.currentHealth();
    }
};

class IsomorphicChillerComparator 
{
    const GameState & _state;

public:

    IsomorphicChillerComparator(const GameState & state)
        : _state(state)
    {
    }

    bool operator() (const IsomorphicCardSet & ics1, const IsomorphicCardSet & ics2) const
    {
        const Card & card1 = ics1.getCurrentCard(_state);
        const Card & card2 = ics2.getCurrentCard(_state);

        return card1.getType().getTargetAbilityAmount() > card2.getType().getTargetAbilityAmount();
    }
};

}