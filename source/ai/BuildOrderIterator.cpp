#include "BuildOrderIterator.h"

using namespace Prismata;

BuildOrderIterator::BuildOrderIterator(const GameState & state)
    : _numBuys(0)
    , _nodesSearched(0)
    , _initialState(state)
    , _cardTypeCount(state.numCardsBuyable(), 0)
    , _print(false)
    , _buyPlayerID(state.getActivePlayer())
    , _buyPlayer(state.getActivePlayer())
{
    _allowedBuyableIndex.reserve(state.numCardsBuyable());
    _boughtCardIDStack.reserve(20);
    
    _buyLimits.setLimit(CardTypes::GetCardType("Forcefield"), 0);
    _buyLimits.setLimit(CardTypes::GetCardType("Animus"), 2);
    _buyLimits.setLimit(CardTypes::GetCardType("Blastforge"), 3);
    _buyLimits.setLimit(CardTypes::GetCardType("Conduit"), 4);
    _buyLimits.setLimit(CardTypes::GetCardType("Engineer"), 5);

    for (size_t i(0); i < state.numCardsBuyable(); ++i)
    {
        // set up the number of cards of each type we already have
        _cardTypeCount[i] = _initialState.numCardsOfType(_initialState.getActivePlayer(), state.getCardBuyableByIndex(i).getType());

        _allowedBuyableIndex.push_back(i);
    }

    PRISMATA_ASSERT(state.numCards(state.getInactivePlayer()) == 0, "Enemy player in the state must have no units, this is a single agent problem");
}

#include "Timer.h"
void BuildOrderIterator::solve()
{
    _numBuys = 0;
    _print = false;

    GameState state(_initialState);

    Timer t;
    t.start();

    _econMove.clear();
    _buyPlayer.getMove(state, _econMove);
    recurse(state, 0, 0, 0);

    double ms = t.getElapsedTimeInMilliSec();
    std::cout << "Build order search completed in " << ms << "ms\n";
}

void BuildOrderIterator::debugSolve()
{
    _numBuys = 0;
    _print = true;
    
    GameState state(_initialState);

    _econMove.clear();
    _buyPlayer.getMove(state, _econMove);
    recurse(state, 0, 0, 0);
}

void BuildOrderIterator::passTurn(const GameState & state, const size_t turn)
{
    GameState nextState(state);

    const Action pass(state.getActivePlayer(), ActionTypes::END_PHASE, 0);
    const Action enemyPass(state.getInactivePlayer(), ActionTypes::END_PHASE, 0);

    // we are in the action phase with no breach targets, so passing twice goes to enemy turn
    nextState.doAction(pass);
    nextState.doAction(pass);

    // enemy has no defenders so we are in action phase, pass twice goes back to buy turn
    nextState.doAction(enemyPass);
    nextState.doAction(enemyPass);

    // activate all economy cards on the purchasing player's side
    _econMove.clear();
    _buyPlayer.getMove(nextState, _econMove);

    recurse(nextState, 0, 0, turn + 1);
}

void BuildOrderIterator::recurse(GameState & state, const CardID currentCardBuyableIndex, const size_t numBought, const size_t turn)
{
    ++_nodesSearched;

    if (_actionStack.size() <= turn)
    {
        _actionStack.push_back(std::vector<Action>());
        _actionStack[turn].reserve(20);
    }

    if (numBought > 3 || turn > 3)
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

    const CardType & cardType = state.getCardBuyableByIndex(_allowedBuyableIndex[currentCardBuyableIndex]).getType();
    const Action buyCard(state.getActivePlayer(), ActionTypes::BUY, cardType.getID());
        
    bool hasBuyLimit = _buyLimits.hasLimit(cardType.getID());
    CardID numOwned = _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]];
    bool buyLimitExceeded = hasBuyLimit && (numOwned >= _buyLimits.getLimit(cardType));

    if (state.isLegal(buyCard) && !buyLimitExceeded)
    {
        // buy the card
        state.doAction(buyCard);
        _boughtCardIDStack.push_back(state.getLastCardBoughtID());
        _actionStack[turn].push_back(buyCard);
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]++;
        
        // call recursion after having bought this card
        recurse(state, currentCardBuyableIndex, numBought + 1, turn);

        // try the option of passing the turn after buying this card
        passTurn(state, turn);

        // sell the card
        _actionStack[turn].pop_back();
        _cardTypeCount[_allowedBuyableIndex[currentCardBuyableIndex]]--;

        const Action sell(buyCard.getPlayer(), ActionTypes::SELL, _boughtCardIDStack.back());
        _boughtCardIDStack.pop_back();

        state.doAction(sell);
    }

    // call recursion after having skipped this card
    recurse(state, currentCardBuyableIndex + 1, 0, turn);
}

size_t BuildOrderIterator::getNodesSearched() 
{
    return _nodesSearched;
}

void BuildOrderIterator::printStack() const
{
    for (size_t i(0); i<_actionStack.size(); ++i)
    {
        for (size_t t(0); t<i; ++t)
        {
            std::cout << "  ";
        }

        for (size_t j(0); j < _actionStack[i].size(); ++j)
        {
            std::cout << ((j > 0) ? ", " : "") << CardTypes::GetAllCardTypes()[_actionStack[i][j].getID()].getName();
        }
                
        std::cout << "\n";
    }   
}

size_t BuildOrderIterator::getNumBuys()
{
    return _numBuys;
}

void BuildOrderIterator::setBuyLimits(const BuyLimits & buyLimits)
{
    _buyLimits = buyLimits;
}
