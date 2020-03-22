#pragma once

#include "Common.h"
#include "PartialPlayer.h"
#include "BuyLimits.h"
#include "rapidjson/document.h"

namespace Prismata
{
    
typedef std::vector< std::pair<CardType, size_t> > BuySequence;

class PartialPlayer_ActionBuy_Sequence : public PartialPlayer
{
    BuySequence     _buySequence;

public:

    PartialPlayer_ActionBuy_Sequence (const PlayerID playerID, const BuySequence & buySequence);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_Sequence(*this));}
};
}