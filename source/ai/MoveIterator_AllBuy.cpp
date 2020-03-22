#include "MoveIterator_AllBuy.h"
#include "AllPlayers.h"
#include "CanonicalOrderComparator.h"

using namespace Prismata;

MoveIterator_AllBuy::MoveIterator_AllBuy(const PlayerID playerID)
    : _movesGenerated(0)
    , _legalMovesGenerated(0)
{
    _hasMoreMoves = false;
    _playerID = playerID;
}

bool MoveIterator_AllBuy::generateNextChild(GameState & child, Move & movePerformed)
{
    if (_state.getActivePhase() != Phases::Action)
    {
        return false;
    }
    else if (_buyableCardIDs.size() == 0 && _hasMoreMoves)
    {
        child = _state;
        const Action endPhase(_playerID, ActionTypes::END_PHASE);

        PRISMATA_ASSERT(child.isLegal(endPhase), "We should be able to end phase after all buys performed");

        movePerformed.clear();
        child.doAction(endPhase);
        movePerformed.addAction(endPhase);

        _hasMoreMoves = false;
        return true;
    }

    while (true)
    {
        movePerformed.clear();

        // if no more moves are possible, return false
        if (!hasMoreMoves())
        {
            return false;
        }

        // the child starts out as a clone of the initial state
        child = _state;
        bool illegalActionFound = false;

        // iterate through each isomorphic card
        for (size_t i(0); i < _currentBuyIndex.size(); ++i)
        {
            // activate the number of them indicated in isoIndex
            for (size_t a(0); a < _currentBuyIndex[i]; ++a)
            {
                const Action action(_playerID, ActionTypes::BUY, _buyableCardIDs[i]);

                // if the action is legal, do it and add it to the move
                if (child.isLegal(action))
                {
                    child.doAction(action);
                    movePerformed.addAction(action);
                }
                // if the action isn't legal then no further actions on this unit will be legal either so exit this loop
                else
                {
                    illegalActionFound = true;
                    break;
                }
            }

            if (illegalActionFound)
            {
                break;       
            }
        }

        /*if (!illegalActionFound)
        {
            printf("%5d | ", _legalMovesGenerated);
            for (size_t i(0); i < _currentBuyIndex.size(); ++i)
            {
                printf("%d ", _currentBuyIndex[i]);
            }   printf("\n");
        }*/

        incrementMove();
        _movesGenerated++;

        if (!illegalActionFound)
        {
            // we should be able to end the phase here
            const Action endPhase(_playerID, ActionTypes::END_PHASE);

            PRISMATA_ASSERT(child.isLegal(endPhase), "We should be able to end phase after all buys performed");

            child.doAction(endPhase);
            movePerformed.addAction(endPhase);

            _legalMovesGenerated++;
            return true;   
        }
    }

    return false;
}

void MoveIterator_AllBuy::setState(const GameState & state)
{
    _state = state;

    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    reset();
}

void MoveIterator_AllBuy::processBuyableCards()
{
    for (size_t i(0); i < _state.numCardsBuyable(); ++i)
    {
        GameState tempState(_state);
        const CardBuyable & cb = _state.getCardBuyableByIndex(i);

        const Action buy(_playerID, ActionTypes::BUY, cb.getType().getID());

        size_t numBuyable = 0;
        while (tempState.isLegal(buy))
        {
            numBuyable++;
            tempState.doAction(buy);
        }

        if (numBuyable > 0)
        {
            _buyableCardIDs.push_back(cb.getType().getID());
            _maxBuyable.push_back(numBuyable);
        }
    }

    _currentBuyIndex = std::vector<size_t>(_buyableCardIDs.size(), 0);

    for (size_t i(0); i < _buyableCardIDs.size(); ++i)
    {
        //printf("%3d | %s\n", _maxBuyable[i], _state.getCardBuyableByID(_buyableCardIDs[i]).getType().getUIName().c_str());
    }

}

void MoveIterator_AllBuy::reset()
{
    _hasMoreMoves = true;
    _movesGenerated = 0;
    _legalMovesGenerated = 0;
    _buyableCardIDs.clear();
    _maxBuyable.clear();

    processBuyableCards();
}

void MoveIterator_AllBuy::incrementMove(const size_t index)
{
    // increment the index for this unit
    _currentBuyIndex[index] = (_currentBuyIndex[index] + 1) % (_maxBuyable[index] + 1);

    // if the value rolled over, we need to do the carry calculation
    if (_currentBuyIndex[index] == 0)
    {
        // if we have space left to increment, do it
        if (index < _currentBuyIndex.size() - 1)
        {
            incrementMove(index + 1);
        }
        // otherwise we have no more moves
        else
        {
            // stop
            _hasMoreMoves = false;
        }
    }
}

MoveIteratorPtr MoveIterator_AllBuy::clone()
{
    MoveIterator_AllBuy * temp = new MoveIterator_AllBuy(*this);
    
    return MoveIteratorPtr(temp);   
}


std::string MoveIterator_AllBuy::getDescription() 
{ 
    return "NOPE";
}