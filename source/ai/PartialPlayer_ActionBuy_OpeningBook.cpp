#include "PartialPlayer_ActionBuy_OpeningBook.h"

using namespace Prismata;

PartialPlayer_ActionBuy_OpeningBook::PartialPlayer_ActionBuy_OpeningBook(const PlayerID playerID, const OpeningBook & openingBook)
{
    _playerID = playerID;
    _phaseID = PPPhases::ACTION_BUY;

    _openingBook = openingBook;
}

void PartialPlayer_ActionBuy_OpeningBook::getMove(GameState & state, Move & move)
{
    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "GameState player does not match PartialPlayer player: %d != %d", (int)state.getActivePlayer(), (int)_playerID);

    if (state.getActivePhase() != Phases::Action)
    {
        return;
    }

    for (size_t i(0); i < _openingBook.size(); ++i)
    {
        if (_openingBook[i].isValid() && _openingBook[i].matchesState(state))
        {
            const std::vector<CardType> & cardsToBuy = _openingBook[i].getCardsToBuy();

            for (size_t c(0); c < cardsToBuy.size(); ++c)
            {
                const Action action(_playerID, ActionTypes::BUY, cardsToBuy[c].getID());

                //PRISMATA_ASSERT(state.isLegal(action), state, "BadState_OpeningBook.png", "Opening book action not legal for this state");

                if (state.isLegal(action))
                {
                    state.doAction(action);
                    move.addAction(action);
                }
            }

            break;
        }
    }
}

bool OpeningBookEntry::isValid() const
{
    return _isValid;
}

bool OpeningBookEntry::matchesState(const GameState & state) const
{
    const PlayerID self = state.getActivePlayer();
    const PlayerID enemy = state.getInactivePlayer();

    CardID selfTotalCards = state.numCards(self);
    CardID enemyTotalCards = state.numCards(self);
    CardID selfMatchTotalCards = 0;
    CardID enemyMatchTotalCards = 0;

    for (size_t c(0); c < _selfCardTypeCounts.size(); ++c)
    {
        selfMatchTotalCards += _selfCardTypeCounts[c];

        const size_t stateCards = state.numCardsOfType(self, _selfCardTypes[c]);
        if (_selfCardTypeCounts[c] != stateCards)
        {
            return false;
        }
    }

    for (size_t c(0); c < _enemyCardTypeCounts.size(); ++c)
    {
        enemyMatchTotalCards += _enemyCardTypeCounts[c];

        if (_enemyCardTypeCounts[c] != state.numCardsOfType(enemy, _enemyCardTypes[c]))
        {
            return false;
        }
    }

    // if our card totals don't match exactly, then it's not a match
    if (state.numCards(self) != selfMatchTotalCards)
    {
        return false;
    }

    // if we care about enemy cards and it's not a match
    if ((enemyMatchTotalCards > 0) && (state.numCards(enemy) != enemyMatchTotalCards))
    {
        return false;
    }

    // check that the cards buyable are present in the state
    for (size_t c(0); c < _buyableCardTypes.size(); ++c)
    {
        bool match = false;
        for (size_t cb(0); cb < state.numCardsBuyable(); ++cb)
        {
            if (state.getCardBuyableByIndex(cb).getType() == _buyableCardTypes[c])
            {
                match = true;
                break;
            }
        }

        if (!match)
        {
            return false;
        }
    }

    return true;
}

const std::vector<CardType> & OpeningBookEntry::getCardsToBuy() const
{
    return _cardsToBuy;
}

OpeningBookEntry::OpeningBookEntry(const rapidjson::Value & openingBookEntryValue)
    : _isValid(true)
{
    PRISMATA_ASSERT(openingBookEntryValue.IsObject(), "Opening book entry must be an object");
    PRISMATA_ASSERT(openingBookEntryValue.HasMember("self") && openingBookEntryValue["self"].IsArray(), "Opening book entry must contain self array");
    PRISMATA_ASSERT(openingBookEntryValue.HasMember("buy") && openingBookEntryValue["buy"].IsArray(), "Opening book entry must contain BuySequence array");

    const rapidjson::Value & self = openingBookEntryValue["self"];
    for (size_t i(0); i < self.Size(); ++i)
    {
        const rapidjson::Value & selfEntry = self[i];

        PRISMATA_ASSERT(selfEntry.IsArray() && selfEntry.Size() == 2 && selfEntry[0u].IsString() && selfEntry[1u].IsInt(), "Self entry should be array of size 2");
            
        // check to see if this card type exists, if it doesn't this isn't valid
        const std::string cardTypeString = selfEntry[0u].GetString();
        if (!CardTypes::CardTypeExists(cardTypeString))
        {
            _isValid = false;
            return;
        }

        const CardType type = CardTypes::GetCardType(cardTypeString);
        _selfCardTypes.push_back(type);
        _selfCardTypeCounts.push_back(selfEntry[1u].GetInt());
    }

    // check to see if we have a non-empty Enemy array
    if (openingBookEntryValue.HasMember("enemy") && openingBookEntryValue["enemy"].IsArray() && (openingBookEntryValue["enemy"].Size() > 0))
    {
        const rapidjson::Value & enemy = openingBookEntryValue["enemy"];
        for (size_t i(0); i < enemy.Size(); ++i)
        {
            const rapidjson::Value & enemyEntry = enemy[i];

            PRISMATA_ASSERT(enemyEntry.IsArray() && enemyEntry.Size() == 2 && enemyEntry[0u].IsString() && enemyEntry[1u].IsInt(), "Self entry should be array of size 2");
            
            // check to see if this card type exists, if it doesn't this isn't valid
            const std::string cardTypeString = enemyEntry[0u].GetString();
            if (!CardTypes::CardTypeExists(cardTypeString))
            {
                _isValid = false;
                return;
            }

            _selfCardTypes.push_back(CardTypes::GetCardType(cardTypeString));
            _selfCardTypeCounts.push_back(enemyEntry[1u].GetInt());
        }
    }

    // check to see if we have a non-empty buyable array
    if (openingBookEntryValue.HasMember("buyable") && openingBookEntryValue["buyable"].IsArray() && (openingBookEntryValue["buyable"].Size() > 0))
    {
        const rapidjson::Value & buyable = openingBookEntryValue["buyable"];
        for (size_t i(0); i < buyable.Size(); ++i)
        {
            PRISMATA_ASSERT(buyable[i].IsString(), "buyable entry should contain String");

            // check to see if this card type exists, if it doesn't this isn't valid
            const std::string cardTypeString = buyable[i].GetString();
            if (!CardTypes::CardTypeExists(cardTypeString))
            {
                _isValid = false;
                return;
            }

            _buyableCardTypes.push_back(CardTypes::GetCardType(buyable[i].GetString()));
        }
    }

    // construct the cards to buy
    const rapidjson::Value & buyCards = openingBookEntryValue["buy"];
    for (size_t i(0); i < buyCards.Size(); ++i)
    {
        PRISMATA_ASSERT(buyCards[i].IsString(), "buy entry should contain String");

        // check to see if this card type exists, if it doesn't this isn't valid
        const std::string cardTypeString = buyCards[i].GetString();
        if (!CardTypes::CardTypeExists(cardTypeString))
        {
            _isValid = false;
            return;
        }

        _cardsToBuy.push_back(CardTypes::GetCardType(buyCards[i].GetString()));
    }
}
