#include "MoveIterator_AllBreach.h"
#include "AllPlayers.h"
#include "CanonicalOrderComparator.h"

using namespace Prismata;

MoveIterator_AllBreach::MoveIterator_AllBreach(const PlayerID playerID)
    : _movesGenerated(0)
    , _legalMovesGenerated(0)
    , _currentMove(0)
    , _canBreachAll(false)
{
    _hasMoreMoves = false;
    _playerID = playerID;
}

bool MoveIterator_AllBreach::generateNextChild(GameState & child, Move & movePerformed)
{
    if (_state.getActivePhase() != Phases::Breach)
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
    
    movePerformed = _allMoves[_currentMove++];
    child.doMove(movePerformed);
    
    const Action endPhase(_playerID, ActionTypes::END_PHASE);
    PRISMATA_ASSERT(child.isLegal(endPhase), "We should be able to end phase after all breach performed");
    child.doAction(endPhase);
    movePerformed.addAction(endPhase);

    if (_currentMove >= _allMoves.size())
    {
        _hasMoreMoves = false;
    }
    
    return true;
}

void MoveIterator_AllBreach::processIsomorphicCards()
{
    PRISMATA_ASSERT(_state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");
    
    // for each card we own, put it into one of the isomorphic card sets
    for (const auto & cardID : _state.getCardIDs(_state.getInactivePlayer()))
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
    _currentFinalBreachIndex = 0;
}

void MoveIterator_AllBreach::generateAllMoves()
{
    _endPhase = Action(_playerID, ActionTypes::END_PHASE);

    // for each isomorphic card we have
    for (size_t i(0); i < _isoCardSets.size() && !_canBreachAll; ++i)
    {
        _finalBreachIndex = i;
        _finalBreachCard = _state.getCardByID(_isoCardSets[_finalBreachIndex].getCardIDs().back());
        _finalBreachAction = Action(_playerID, ActionTypes::ASSIGN_BREACH, _finalBreachCard.getID());
        _finalBreachActionUndo = Action(_playerID, ActionTypes::UNDO_BREACH, _finalBreachCard.getID());
        
        //printf("Final: %d\n", _finalBreachCard.getID());
        checkSolution();

        // generate all breach sequences with it as the final breach target
        recurse(0);
    }
}

bool MoveIterator_AllBreach::isTerminalState(size_t isoIndex)
{
    if (isoIndex >= _isoCardSets.size())
    {
        return true;
    }

    if (_canBreachAll)
    {
        return true;
    }

    // if we can end the phase here then we can't breach our last target so we prune
    if (_currentState.isLegal(_endPhase))
    {
        return true;
    }

    // if we don't have enough damage to breach the final card then prune here
    //if (_finalBreachCard.currentHealth() > _currentState.getAttack(_playerID) && !_finalBreachCard.getType().isFragile())
    //{
    //    return true;
    //}

    // if we are trying to breach more of something than exists
    size_t currentIsoMax = (isoIndex == _finalBreachIndex) ? (_isoCardSets[isoIndex].size() - 1) : _isoCardSets[isoIndex].size();
    if (_currentIsoIndex[isoIndex] > currentIsoMax)
    {
        return true;
    }

    return false;
}

bool MoveIterator_AllBreach::checkSolution()
{
    bool solution = false;

    if (!_currentState.isLegal(_finalBreachAction))
    {
        return false;
    }

    // do the final breach action
    _currentState.doAction(_finalBreachAction);

    // if we can end the phase then this is a solution
    if (_currentState.isLegal(_endPhase))
    {
        solution = true;
        _allMoves.push_back(_actionStack);
        _allMoves.back().addAction(_finalBreachAction);

        if (_currentState.numCards(_currentState.getInactivePlayer()) == 0)
        {
            _canBreachAll = true;
        }

        /*printf("* ");
        for (size_t i(0); i < _allMoves.back().size(); ++i)
        {
            printf("%3d", _allMoves.back().getAction(i).getID());
        }   printf("\n");*/
    }

    _currentState.doAction(_finalBreachActionUndo);

    return solution;
}

void MoveIterator_AllBreach::recurse(size_t isoIndex)
{
    if (isTerminalState(isoIndex))
    {
        return;
    }
    
    // implement the current breach
    
    const std::vector<CardID> & cardIDs = _isoCardSets[isoIndex].getCardIDs();
    size_t cardIndex = _currentIsoIndex[isoIndex];
    
    if (cardIndex < cardIDs.size())
    {
        const CardID breachCardID = cardIDs[cardIndex];
        const Action breach(_playerID, ActionTypes::ASSIGN_BREACH, breachCardID);

        if (_currentState.isLegal(breach))
        {
            // do the action on the state
            _currentState.doAction(breach);
            _actionStack.addAction(breach);
        
            checkSolution();

            _currentIsoIndex[isoIndex]++;

            // recurse branch where we breach this iso type again
            recurse(isoIndex);

            // unroll the moves we did above
            const Action undoBreach(_playerID, ActionTypes::UNDO_BREACH, breachCardID);
            _currentState.doAction(undoBreach);
            _actionStack.popAction();
            _currentIsoIndex[isoIndex]--;
        }
    }

    // recurse breanch with this card having been skipped
    recurse(isoIndex + 1);
}

void MoveIterator_AllBreach::setState(const GameState & state)
{
    _state = state;
    _currentState = state;

    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    reset();
}

void MoveIterator_AllBreach::reset()
{
    _hasMoreMoves = true;
    _movesGenerated = 0;
    _legalMovesGenerated = 0;
    _currentFinalBreachIndex = 0;
    _currentMove = 0;
    _canBreachAll = false;
    _isoCardSets.clear();
    _allMoves.clear();
    
    processIsomorphicCards();

    if (_isoCardSets.size() == 0)
    {
        _hasMoreMoves = false;
        return;
    }

    generateAllMoves();

    if (_allMoves.empty())
    {
        _hasMoreMoves = false;
    }
}

MoveIteratorPtr MoveIterator_AllBreach::clone()
{
    MoveIterator_AllBreach * temp = new MoveIterator_AllBreach(*this);
    
    return MoveIteratorPtr(temp);   
}

std::string MoveIterator_AllBreach::getDescription() 
{ 
    return "NOPE";
}
