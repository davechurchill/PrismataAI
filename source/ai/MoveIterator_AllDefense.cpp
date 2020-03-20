#include "MoveIterator_AllDefense.h"
#include "AllPlayers.h"
#include "CanonicalOrderComparator.h"

using namespace Prismata;

MoveIterator_AllDefense::MoveIterator_AllDefense(const PlayerID & playerID)
    : _movesGenerated(0)
    , _legalMovesGenerated(0)
{
    _hasMoreMoves = false;
    _playerID = playerID;
}

bool MoveIterator_AllDefense::generateNextChild(GameState & child, Move & movePerformed)
{
    if (_state.getActivePhase() != Phases::Defense)
    {
        return false;
    }

    movePerformed.clear();

    // if no more moves are possible, return false
    if (!hasMoreMoves())
    {
        return false;
    }

    child = _state;

    /*printf("%d | ", _currentFinalBlockerIndex);
    for (size_t i(0); i < _currentIsoIndex.size(); ++i)
    {
        printf("%d ", _currentIsoIndex[i]);
    }   printf("\n");*/

    // generate and do the actions
    for (size_t i(0); i < _isoCardSets.size(); ++i)
    {
        for (size_t a(0); a < _currentIsoIndex[i]; ++a)
        {
            const Action action(_playerID, ActionTypes::ASSIGN_BLOCKER, _isoCardSets[i].getCardIDs()[a]);

            PRISMATA_ASSERT(child.isLegal(action), "Block action should be legal");

            child.doAction(action);
            movePerformed.addAction(action);
        }
    }

    // do the final block
    const Action finalBlock(_playerID, ActionTypes::ASSIGN_BLOCKER, _isoCardSets[_currentFinalBlockerIndex].getCardIDs().back());
    PRISMATA_ASSERT(child.isLegal(finalBlock), "Final blocker not legal");

    child.doAction(finalBlock);
    movePerformed.addAction(finalBlock);

    const Action endPhase(_playerID, ActionTypes::END_PHASE);

    PRISMATA_ASSERT(child.isLegal(endPhase), "We should be able to end phase after all blocks performed");

    child.doAction(endPhase);
    movePerformed.addAction(endPhase);

    
    // if the current sequence isn't legal then try the next one
    do
    {
        incrementMove();
    }
    while (!isBlockingSequenceLegal() && _hasMoreMoves);

    return true;
}

bool MoveIterator_AllDefense::isBlockingSequenceLegal() const
{
    size_t incomingDamage = _state.getAttack(_state.getInactivePlayer());

    size_t blockingAssigned = 0;
    for (size_t i(0); i < _isoCardSets.size(); ++i)
    {
        blockingAssigned += _currentIsoIndex[i] * _state.getCardByID(_isoCardSets[i].getCardIDs()[0]).currentHealth();
    }

    size_t finalBlockerHealth = _state.getCardByID(_isoCardSets[_currentFinalBlockerIndex].getCardIDs()[0]).currentHealth();

    return (blockingAssigned < incomingDamage) && (blockingAssigned + finalBlockerHealth >= incomingDamage);
}

void MoveIterator_AllDefense::processIsomorphicCards()
{
    PRISMATA_ASSERT(_state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    std::vector<CardID> blockingCardIDs;

    for (const auto & cardID : _state.getCardIDs(_playerID))
    {
        const Card & card = _state.getCardByID(cardID);

        // if this card type doesn't have an activated ability then continue
        if (!card.canBlock())
        {
            continue;
        }

        blockingCardIDs.push_back(cardID);
    }

    // for each card we own, put it into one of the isomorphic card sets
    for (const auto & cardID : blockingCardIDs)
    {
        bool found = false;
        for (auto & isoCardSet : _isoCardSets)
        {
            if (isoCardSet.isIsomorphic(_state, cardID))
            {
                found = true;
                isoCardSet.add(cardID);
                break;
            }
        }

        if (!found)
        {
            _isoCardSets.push_back(IsomorphicCardSet());
            _isoCardSets.back().add(cardID);

            //printf("Isomorphic Card: %s\n", _state.getCardByID(cardID).getType().getUIName().c_str());
        }
    }

    _currentIsoIndex = std::vector<size_t>(_isoCardSets.size(), 0);
    _currentFinalBlockerIndex = 0;
}

void MoveIterator_AllDefense::setState(const GameState & state)
{
    _state = state;

    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    reset();
}


void MoveIterator_AllDefense::getRandomMove(const GameState & state, Move & move)
{

}

void MoveIterator_AllDefense::reset()
{
    _hasMoreMoves = true;
    _movesGenerated = 0;
    _legalMovesGenerated = 0;
    _currentFinalBlockerIndex = 0;
    _isoCardSets.clear();
    processIsomorphicCards();

    if (_isoCardSets.empty())
    {
        _hasMoreMoves = false;
    }

    // increment the initial move state since 0 blockers is never legal
    while (_hasMoreMoves && !isBlockingSequenceLegal())
    {
        incrementMove();
    }
}

void MoveIterator_AllDefense::incrementMove(const size_t index)
{
    // if this index is the current final blocker then we need to remove one possible card from this index
    size_t currentIndexLimit = (index == _currentFinalBlockerIndex) ? _isoCardSets[index].size() : _isoCardSets[index].size() + 1;

    // increment the index for this unit
    _currentIsoIndex[index] = (_currentIsoIndex[index] + 1) % currentIndexLimit;

    // if this blocking sequence is not legal then incrementing this card index again won't be legal
    // so we set this index back to 0 and let it roll over to the next value
    // this is a pruning heuristic and isn't required for this to work but it saves a lot of iterations
    //if (!isBlockingSequenceLegal())
    //{
    //    _currentIsoIndex[index] = 0;
    //}

    // if the value rolled over, we need to do the carry calculation
    if (_currentIsoIndex[index] == 0)
    {
        // if we have space left to increment, do it
        if (index < _currentIsoIndex.size() - 1)
        {
            incrementMove(index + 1);
        }
        // otherwise we have no more moves with this final blocker
        else
        {
            // go to the next final blocker
            if (_currentFinalBlockerIndex < _isoCardSets.size() - 1)
            {
                _currentFinalBlockerIndex++;
            }
            else
            {
                // we've tried everything
                _hasMoreMoves = false;
                return;
            }
        }
    }
}

MoveIteratorPtr MoveIterator_AllDefense::clone()
{
    MoveIterator_AllDefense * temp = new MoveIterator_AllDefense(*this);
    
    return MoveIteratorPtr(temp);   
}


std::string MoveIterator_AllDefense::getDescription() 
{ 
    return "NOPE";
}