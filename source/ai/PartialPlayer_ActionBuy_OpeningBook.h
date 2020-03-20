#pragma once

#include "Common.h"
#include "PartialPlayer.h"

namespace Prismata
{

class OpeningBookEntry
{
    std::vector<CardType>   _selfCardTypes;
    std::vector<size_t>     _selfCardTypeCounts;

    std::vector<CardType>   _enemyCardTypes;
    std::vector<size_t>     _enemyCardTypeCounts;

    std::vector<CardType>   _buyableCardTypes;

    std::vector<CardType>   _cardsToBuy;

    bool                    _isValid;

public:

    OpeningBookEntry(const rapidjson::Value & openingBookEntryValue);

    bool matchesState(const GameState & state) const;

    const std::vector<CardType> & getCardsToBuy() const;

    bool isValid() const;
};

typedef std::vector<OpeningBookEntry> OpeningBook;

class PartialPlayer_ActionBuy_OpeningBook : public PartialPlayer
{
    OpeningBook _openingBook;

public:

    PartialPlayer_ActionBuy_OpeningBook (const PlayerID & playerID, const OpeningBook & openingBook);
    void getMove(GameState & state, Move & move);

    PPPtr clone() { return PPPtr(new PartialPlayer_ActionBuy_OpeningBook(*this));}
};
}