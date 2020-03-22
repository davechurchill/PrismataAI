#include "MoveIterator_All.h"
#include "AllPlayers.h"
#include "CanonicalOrderComparator.h"

#include "MoveIterator_AllDefense.h"
#include "MoveIterator_AllAbility.h"
#include "MoveIterator_AllBuy.h"
#include "MoveIterator_AllBreach.h"

using namespace Prismata;

MoveIterator_All::MoveIterator_All(const PlayerID playerID)
    : _movesGenerated(0)
    , _legalMovesGenerated(0)
{
    _hasMoreMoves = false;
    _playerID = playerID;
}

bool MoveIterator_All::generateNextChild(GameState & child, Move & movePerformed)
{
    movePerformed.clear();

    if (!hasMoreMoves())
    {
        return false;
    }

    _movesGenerated++;

    child = _states.back();
    
    //printf("%5d  ", _movesGenerated);
    for (size_t i(0); i < _iterators.size(); ++i)
    {
        for (size_t a(0); a < _moves[i].size(); ++a)
        {
            if (_moves[i].getAction(a).getType() != ActionTypes::END_PHASE)
            {
                //printf("%d ", _moves[i].getAction(a).getID());
            }
        }
        //printf("| ");

        movePerformed.addMove(_moves[i]);
    }   //printf("\n");

    incrementMove(_iterators.size() - 1);

    // keep passing until the turn ends
    const Action endPhase(_playerID, ActionTypes::END_PHASE);
    while (child.getActivePlayer() == _playerID)
    {
        PRISMATA_ASSERT(child.isLegal(endPhase), "Should be able to end phase here");

        child.doAction(endPhase);
        movePerformed.addAction(endPhase);
    }

    return true;
}

void MoveIterator_All::setState(const GameState & state)
{
    _state = state;

    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    reset();
}

void MoveIterator_All::incrementMove(const size_t index)
{
    _moves[index].clear();

    /*printf("%d  ", _movesGenerated);
    for (size_t i(0); i < index; ++i)
    {
        printf("  ");
    }

    printf("%d\n", index);*/

    // increment the iterator
    bool moveGenerated = _iterators[index]->generateNextChild(_states[index], _moves[index]);

    // if the iterator did not generate a move, roll-over to the iterator to the left
    if (!moveGenerated)
    {
        // if we're not at the left-moves iterator, recurse to the left
        if (index > 0)
        {
            incrementMove(index - 1);
        }
        // otherwise we have no more moves to generate
        else
        {
            // stop
            _hasMoreMoves = false;
            return;
        }
    }
    
    // if we generated a move and this is the final roll-over
    // we need to generate every first move to the right
    for (size_t i(index+1); i < _iterators.size(); ++i)
    {
        _moves[i].clear();
        _iterators[i]->setState(_states[i-1]);

        // try to generate a child
        if (!_iterators[i]->generateNextChild(_states[i], _moves[i]))
        {
            // if no child was generated then the phase doesn't exist, so copy the previous state and clear the move
            _states[i] = _states[i-1];
            _moves[i].clear();
        }
    }
}

void MoveIterator_All::reset()
{
    _hasMoreMoves = true;
    _movesGenerated = 0;
    _legalMovesGenerated = 0;
        
    _states = std::vector<GameState>(4);
    _moves = std::vector<Move>(4);

    _iterators.clear();
    _iterators.push_back(MoveIteratorPtr(new MoveIterator_AllDefense(_playerID)));
    _iterators.push_back(MoveIteratorPtr(new MoveIterator_AllAbility(_playerID)));
    _iterators.push_back(MoveIteratorPtr(new MoveIterator_AllBuy(_playerID)));
    _iterators.push_back(MoveIteratorPtr(new MoveIterator_AllBreach(_playerID)));

    for (size_t i(0); i < _iterators.size(); ++i)
    {
        const GameState & state = (i > 0) ? _states[i-1] : _state;
        
        _iterators[i]->setState(state);
        
        if (!_iterators[i]->generateNextChild(_states[i], _moves[i]))
        {
            _states[i] = state;
            _moves[i].clear();
        }
    }

    //processFutureIterators(0);
}

//void MoveIterator_All::processFutureIterators(size_t index)
//{
//    GameState child(_state);
//    Move m;
//
//    while (_currentIterators.size() > index)
//    {
//        _currentIterators.pop_back();
//    }
//
//    // determine which iterators are relevant
//    for (size_t i(index); i < 4; ++i)
//    {
//        GameState & state = (i > 0) ? _states[i-1] : _state;
//
//        MoveIteratorPtr currentIterator = _iterators[i]->clone();
//        bool generatedChild = currentIterator->generateNextChild(child, m);
//
//        if (generatedChild)
//        {
//            _iterators.push_back(currentIterator);
//            _states.
//        }
//
//        parent = child;
//    }
//}

MoveIteratorPtr MoveIterator_All::clone()
{
    MoveIterator_All * temp = new MoveIterator_All(*this);
    
    return MoveIteratorPtr(temp);   
}

std::string MoveIterator_All::getDescription() 
{ 
    return "NOPE";
}