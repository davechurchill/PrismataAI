#include "BuyIterator.h"

using namespace Prismata;

BuyIterator::BuyIterator(const GameState & state)
    : _numBuys(0)
    , _nodesSearched(0)
    , _state(state)
    , _cardTypeCount(state.numCardsBuyable(), 0)
    , _print(false)
{
    _allowedBuyableIndex.reserve(state.numCardsBuyable());
    _boughtCardIDStack.reserve(20);
    _actionStack.reserve(20);

    for (size_t i(0); i < state.numCardsBuyable(); ++i)
    {
        // set up the number of cards of each type we already have
        _cardTypeCount[i] = _state.numCardsOfType(_state.getActivePlayer(), state.getCardBuyableByIndex(i).getType());

        _allowedBuyableIndex.push_back(i);
    }
}

void BuyIterator::debugSolve()
{
    _numBuys = 0;
    _print = true;
    recurse(0, 0);
}

void BuyIterator::solve()
{
    _numBuys = 0;
    _print = false;
    recurse(0,0);
}

size_t BuyIterator::getNumBuys()
{
    return _numBuys;
}

void BuyIterator::setBuyLimits(const BuyLimits & buyLimits)
{
    _buyLimits = buyLimits;
}

void BuyIterator::recurse(const CardID currentCardBuyableIndex, const size_t numBought)
{
    ++_nodesSearched;

    if (numBought > 2)
    {
        return;
    }

    if (currentCardBuyableIndex >= _allowedBuyableIndex.size())
    {
        if (!_actionStack.empty())
        {
            ++_numBuys;
        }
        
        if (_print)
        {
            printStack();
        }

        return;
    }

    const CardType & cardType = _state.getCardBuyableByIndex(_allowedBuyableIndex[currentCardBuyableIndex]).getType();
    const Action buyCard(_state.getActivePlayer(), ActionTypes::BUY, cardType.getID());
        
    bool hasBuyLimit = _buyLimits.hasLimit(cardType.getID());
    CardID numOwned = _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]];
    bool buyLimitExceeded = hasBuyLimit && (numOwned >= _buyLimits.getLimit(cardType));

    if (_state.isLegal(buyCard) && !buyLimitExceeded)
    {
        // buy the card
        _state.doAction(buyCard);
        _boughtCardIDStack.push_back(_state.getLastCardBoughtID());
        _actionStack.push_back(buyCard);
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]++;
        
        // call recursion after having bought this card
        recurse(currentCardBuyableIndex, numBought + 1);

        // sell the card
        _actionStack.pop_back();
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]--;

        const Action sell(buyCard.getPlayer(), ActionTypes::SELL, _boughtCardIDStack.back());
        _boughtCardIDStack.pop_back();

        PRISMATA_ASSERT(_state.isLegal(sell), "Should be able to sell what we just bought");
        _state.doAction(sell);
    }

    // call recursion after having skipped this card
    recurse(currentCardBuyableIndex + 1, 0);
}

size_t BuyIterator::getNodesSearched() 
{
    return _nodesSearched;
}

void BuyIterator::printStack() const
{
    for (size_t i(0); i<_actionStack.size(); ++i)
    {
        std::cout << ((i > 0) ? ", " : " ") << CardTypes::GetAllCardTypes()[_actionStack[i].getID()].getName();
    }   std::cout << "\n";
}