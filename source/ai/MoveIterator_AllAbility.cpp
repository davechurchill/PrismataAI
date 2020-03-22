#include "MoveIterator_AllAbility.h"
#include "AllPlayers.h"
#include "CanonicalOrderComparator.h"

using namespace Prismata;

MoveIterator_AllAbility::MoveIterator_AllAbility(const PlayerID playerID)
    : _movesGenerated(0)
    , _legalMovesGenerated(0)
    , _currentMove(0)
{
    _hasMoreMoves = false;
    _playerID = playerID;
}

bool MoveIterator_AllAbility::generateNextChild(GameState & child, Move & movePerformed)
{
    if (_state.getActivePhase() != Phases::Action)
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

    if (_currentMove >= _allMoves.size())
    {
        _hasMoreMoves = false;
    }

    return true;
}

void MoveIterator_AllAbility::generateAllMoves()
{
    recurse(0);
}

bool MoveIterator_AllAbility::isTerminalState(size_t isoIndex)
{
    if (isoIndex >= _isoCardSets.size())
    {
        return true;
    }

    // if we are trying to breach more of something than exists
    if (_currentIsoIndex[isoIndex] > _isoCardSets[isoIndex].size())
    {
        return true;
    }

    return false;
}

void MoveIterator_AllAbility::recurse(size_t isoIndex)
{
    if (isTerminalState(isoIndex))
    {
        return;
    }
    
    const std::vector<CardID> & cardIDs = _isoCardSets[isoIndex].getCardIDs();
    size_t cardIndex = _currentIsoIndex[isoIndex];
    
    if (cardIndex < cardIDs.size())
    {
        const CardID breachCardID = cardIDs[cardIndex];
        const Action useAbility(_playerID, ActionTypes::USE_ABILITY, breachCardID);

        if (_currentState.isLegal(useAbility))
        {
            // do the action on the state
            _currentState.doAction(useAbility);
            _actionStack.addAction(useAbility);
        
            _allMoves.push_back(_actionStack);

            _currentIsoIndex[isoIndex]++;

            // recurse branch where we breach this iso type again
            recurse(isoIndex);

            // unroll the moves we did above
            const Action undoAbility(_playerID, ActionTypes::UNDO_USE_ABILITY, breachCardID);
            _currentState.doAction(undoAbility);
            _actionStack.popAction();
            _currentIsoIndex[isoIndex]--;
        }
    }

    // recurse breanch with this card having been skipped
    recurse(isoIndex + 1);
}

void MoveIterator_AllAbility::processIsomorphicCards()
{
    PRISMATA_ASSERT(_state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    std::vector<CardID> abilityCardIDs;

    for (const auto & cardID : _state.getCardIDs(_playerID))
    {
        const Card & card = _state.getCardByID(cardID);

        // if this card type doesn't have an activated ability then continue
        if (!card.getType().hasAbility() || (card.getStatus() == CardStatus::Assigned))
        {
            continue;
        }

        abilityCardIDs.push_back(cardID);
    }

    // sort the cards so that they're in canonical order for activation
    std::sort(abilityCardIDs.begin(), abilityCardIDs.end(), CanonicalOrderComparator(_state));

    // for each card we own, put it into one of the isomorphic card sets
    for (const auto & cardID : abilityCardIDs)
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
}

void MoveIterator_AllAbility::setState(const GameState & state)
{
    _state = state;
    _currentState = state;

    PRISMATA_ASSERT(state.getActivePlayer() == _playerID, "Player to move doesn't match mover iterator playerID");

    reset();
}
void MoveIterator_AllAbility::reset()
{
    _hasMoreMoves = true;
    _movesGenerated = 0;
    _legalMovesGenerated = 0;
    _currentMove = 0;
    _isoCardSets.clear();
    _allMoves.clear();
    processIsomorphicCards();
    
    generateAllMoves();
    _allMoves.push_back(Move());

    std::sort(_allMoves.begin(), _allMoves.end(), MoveLengthComparator());
}

MoveIteratorPtr MoveIterator_AllAbility::clone()
{
    MoveIterator_AllAbility * temp = new MoveIterator_AllAbility(*this);
    
    return MoveIteratorPtr(temp);   
}


std::string MoveIterator_AllAbility::getDescription() 
{ 
    return "NOPE";
}
