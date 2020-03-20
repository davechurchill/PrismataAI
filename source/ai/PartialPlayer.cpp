#include "PartialPlayer.h"

using namespace Prismata;

void PartialPlayer::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(false, "Base PartialPlayer::getMove called for some reason");
}

const PlayerID PartialPlayer::playerID() const
{ 
    return _playerID; 
}

//EnumType PartialPlayer::getPhase() const
//{
//    return _phaseID;
//}

void PartialPlayer::setBuyLimits(const BuyLimits & buyLimits)
{
    _buyLimits = buyLimits;
}

const BuyLimits & PartialPlayer::getBuyLimits()
{
    return _buyLimits;
}

std::string PartialPlayer::getDescription(size_t depth) 
{ 
    std::string s;

    for (size_t i(0); i < depth; ++i)
    {
        s += " ";
    }

    s += _description;

    return s; 
}