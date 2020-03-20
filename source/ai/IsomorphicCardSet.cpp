#include "IsomorphicCardSet.h"

using namespace Prismata;

IsomorphicCardSet::IsomorphicCardSet()
    : _numUsed(0)
{
    _cardIDs.reserve(10);
}
    
bool IsomorphicCardSet::isIsomorphic(const GameState & state, const CardID & otherCardID) const
{
    if (_cardIDs.empty())
    {
        return false;
    }

    return state.getCardByID(_cardIDs[0]).isIsomorphic(state.getCardByID(otherCardID));
}

const std::vector<CardID> & IsomorphicCardSet::getCardIDs() const
{
    return _cardIDs;
}

void IsomorphicCardSet::add(const CardID & cardID)
{
    _cardIDs.push_back(cardID);
}

void IsomorphicCardSet::incUsed()
{
    _numUsed++;
}

void IsomorphicCardSet::decUsed()
{
    _numUsed--;
}

const size_t IsomorphicCardSet::size() const
{
    return _cardIDs.size();
}

const size_t & IsomorphicCardSet::numUsed() const
{
    return _numUsed;
}

const Card & IsomorphicCardSet::getCurrentCard(const GameState & state) const
{
    return state.getCardByID(_cardIDs[_numUsed]);
}

const CardID & IsomorphicCardSet::getCurrentCardID() const
{
    return _cardIDs[_numUsed];
}

bool IsomorphicCardSet::allUsed() const
{
    return _numUsed >= _cardIDs.size();
}