#include "StateChillIterator.h"
#include "Timer.h"
#include "Eval.h"
#include "AITools.h"

using namespace Prismata;
const int SCI_TIMEOUT = -1;
const int SCI_MAX_NODES_REACHED = -2;

StateChillIterator::StateChillIterator(const GameState & initialState)
    : _initialState(initialState)
    , _solved(false)
    , _nodesSearched(0)
    , _numFrozen(0)
    , _maxIterations(0)
    , _player(initialState.getActivePlayer())
    , _enemy(initialState.getInactivePlayer())
{
    _initialStateEval = Eval::WillScoreSum(_initialState, _player);
}

void StateChillIterator::reset()
{
    _actionStack.clear();
    _currentState = _initialState;
    _chillers.clear();
    _blockers.clear();
    _isomorphicChillers.clear();
    _isomorphicBlockers.clear();
    _allStates.clear();
    _bestState = _initialState;
    _bestEval = StateChillEvaluation();

    // add all the chillers to the vector
    for (const CardID & cardID : _initialState.getCardIDs(_player))
    {
        const Card & card = _initialState.getCardByID(cardID);

        if (card.getType().hasTargetAbility() && card.getType().getTargetAbilityType() == ActionTypes::CHILL && !card.isUnderConstruction())
        {
            // add this card to the all chiller set
            _chillers.push_back(cardID);

            // add this card to the correct isomorphic chiller set
            bool isoChillerSetFound = false;
            for (IsomorphicCardSet & ics : _isomorphicChillers)
            {
                if (ics.isIsomorphic(_initialState, cardID))
                {
                    ics.add(cardID);
                    isoChillerSetFound = true;
                }
            }

            // if no isomorphic chillers were found add it to its own set
            if (!isoChillerSetFound)
            {
                _isomorphicChillers.push_back(IsomorphicCardSet());
                _isomorphicChillers.back().add(cardID);
            }
        }
    }

    // add all the enemy blockers to the vector
    for (const CardID & cardID : _initialState.getCardIDs(_enemy))
    {
        const Card & card = _initialState.getCardByID(cardID);

        if (card.canBlock() && !card.isFrozen())
        {
            _blockers.push_back(cardID);

            // add this card to the correct isomorphic blocker set
            bool isoBlockerSetFound = false;
            for (IsomorphicCardSet & ics : _isomorphicBlockers)
            {
                if (ics.isIsomorphic(_initialState, cardID))
                {
                    ics.add(cardID);
                    isoBlockerSetFound = true;
                }
            }

            // if no isomorphic blocker were found add it to its own set
            if (!isoBlockerSetFound)
            {
                _isomorphicBlockers.push_back(IsomorphicCardSet());
                _isomorphicBlockers.back().add(cardID);
            }
        }
    }

    // sort the chillers so that the highest chill amount ones are tried first
    std::sort(_isomorphicChillers.begin(), _isomorphicChillers.end(), IsomorphicChillerComparator(_initialState));

    // sort the blockers so that the highest hp blockers are chilled first
    std::sort(_isomorphicBlockers.begin(), _isomorphicBlockers.end(), IsomorphicBlockerComparator(_initialState));

    // figure out the current best solution of doing nothing
    evaluateState(_initialState, _actionStack);
}

void StateChillIterator::solve(size_t maxIterations)
{
    reset();   
    _maxIterations = maxIterations;
    //printf("\n\n");
    //printf("  #   ChillCost      Damage        Eval | Chill Sequence\n----------------------------------------------\n");

    //printDebug();

    Timer t;
    t.start();

    try
    {
        recurseIsomorphic(0, 0, false);
        _solved = true;
    }
    catch (int e)
    {
        if (e == SCI_MAX_NODES_REACHED || e == SCI_MAX_NODES_REACHED)
        {
            _solved = false;
        }
    }

    double ms = t.getElapsedTimeInMilliSec();
    //printf("\nFound %d nodes, %d freeze combinations, %d unique, in %lf ms\n\n", _nodesSearched, _numFrozen, _allStateIsomorphisms.size(), ms);
    evaluateState(_bestState, _bestMove);
}


bool StateChillIterator::isSolved() const
{
    return _solved;
}

const size_t & StateChillIterator::getNodesSearched() const
{
    return _nodesSearched;
}

// this will recurse over every isomprhic chill sequence (with some duplicates that are hard to eliminate)
void StateChillIterator::recurseIsomorphic(const size_t currentBlockerIndex, const size_t currentChillerIndex, bool partialChill)
{
    ++_nodesSearched;

    // if we have a max node limit for the search and it's hit, throw an exception to stop the search
    if (_maxIterations > 0 && _numFrozen >= _maxIterations)
    {
        throw SCI_MAX_NODES_REACHED;
    }

    // stopping condition is when we've run out of blockers to target
    if (currentBlockerIndex >= _isomorphicBlockers.size())
    {
        return;
    }

    // the min and max index of the blockers we will look over at this depth
    HealthType blockerIndexMin = (HealthType)currentBlockerIndex;
    HealthType blockerIndexMax = partialChill ? blockerIndexMin + 1 : (HealthType)_isomorphicBlockers.size();
        
    // iterate first through every isomorphic locker set the enemy has remaining
    for (CardID blockerIndex(blockerIndexMin); blockerIndex < blockerIndexMax; ++blockerIndex)
    {
        // if we have already used all the isomorphic blockers in this set, continue
        if (_isomorphicBlockers[blockerIndex].numUsed() >= _isomorphicBlockers[blockerIndex].size())
        {
            continue;
        }

        // grab the next card from this isomorphic set
        const Card & blocker = _isomorphicBlockers[blockerIndex].getCurrentCard(_currentState);

        HealthType chillerIndexMin = (HealthType)currentChillerIndex;
        HealthType chillerIndexMax = (HealthType)_isomorphicChillers.size();

        // iterate over choice of isomorphic chillers to chill this unit with
        for (CardID chillerIndex(chillerIndexMin); chillerIndex < chillerIndexMax; ++chillerIndex)
        {
            // if we don't have any chillers of this size left, continue
            if (_isomorphicChillers[chillerIndex].numUsed() >= _isomorphicChillers[chillerIndex].size())
            {
                continue;
            }

            const Card & chiller = _isomorphicChillers[chillerIndex].getCurrentCard(_currentState);

            bool chilledTarget = chillTarget(chiller, chillerIndex, blocker, blockerIndex);
            if (!chilledTarget)
            {
                continue;
            }

            bool unique = true;
            
            // if this is a unique freeze combination then evaluate it
            if (blocker.isFrozen())
            {
                unique = checkStateIsomorphism();

                if (unique)
                {
                    _numFrozen++;
                    evaluateState(_currentState, _actionStack);
                }
            }

            // if the blocker is finished being frozen
            if (blocker.isFrozen())
            {
                // only recurse further if the current state is unique
                if (unique)
                {
                    recurseIsomorphic(blockerIndex, 0, false);
                }
            }
            // don't worry about uniqueness for non-frozen states
            else
            {
                // recurse onto the same card since it's the only legal target
                recurseIsomorphic(blockerIndex, chillerIndex, true);
            }

            // rewind the state of the game and the search to before the action was implemented
            undoChillTarget(chiller, chillerIndex, blocker, blockerIndex);
        }
    }    
}

// chills blocker with chiller on current state, returns whether it was loegal
bool StateChillIterator::chillTarget(const Card & chiller, const CardID & chillerIndex, const Card & blocker, const CardID & blockerIndex)
{
    const Action clickChiller(_player, ActionTypes::USE_ABILITY, chiller.getID());
    const Action chillUnit(_player, ActionTypes::CHILL, chiller.getID(), blocker.getID());
            
    // if this action isn't legal, continue on to the next one
    if (_currentState.isLegal(clickChiller))
    {
        _currentState.doAction(clickChiller);
        _actionStack.addAction(clickChiller);

        _currentState.doAction(chillUnit);
        _actionStack.addAction(chillUnit);

        _isomorphicChillers[chillerIndex].incUsed();
        _currentStateIsomorphism.incChiller(chillerIndex);

        if (blocker.isFrozen())
        {
            _isomorphicBlockers[blockerIndex].incUsed();
            _currentStateIsomorphism.incBlocker(blockerIndex);
        }

        return true;
    }
    
    return false;
}

// chills blocker with chiller on current state, returns whether it was loegal
void StateChillIterator::undoChillTarget(const Card & chiller, const CardID & chillerIndex, const Card & blocker, const CardID & blockerIndex)
{
    // if we are un-freezing the blocker then decrement the number used
    if (blocker.isFrozen())
    {
        _currentStateIsomorphism.decBlocker(blockerIndex);
        _isomorphicBlockers[blockerIndex].decUsed();
    }

    // rewind the state of the game and the search to before the action was implemented
    const Action chillUnitUndo(_player, ActionTypes::UNDO_USE_ABILITY, chiller.getID());
    _currentState.doAction(chillUnitUndo);
    _isomorphicChillers[chillerIndex].decUsed();
    _currentStateIsomorphism.decChiller(chillerIndex);
    _actionStack.popAction();
    _actionStack.popAction();
}

void StateChillIterator::evaluateState(const GameState & state, const Move & move)
{
    GameState currentState(state);

    double currentStateEval = Eval::WillScoreSum(currentState, _player);
    double enemyBlockLoss = AITools::CalculateEnemyNextTurnDefenseLoss(currentState);
    
    double freezeCost = _initialStateEval - currentStateEval;
    double delta = enemyBlockLoss - freezeCost;

    StateChillEvaluation eval(delta, move.size());

    if (eval.betterThan(_bestEval))
    {
        _bestEval = eval;
        _bestState = state;
        _bestMove = move;
    }

    //printf("%3d | %9.5lf | %9.5lf | %9.5lf | ", _numFrozen, freezeCost, enemyBlockLoss, enemyBlockLoss - freezeCost);
    //printActions(move);
}

void StateChillIterator::printDebug()
{
    for (IsomorphicCardSet & ics : _isomorphicBlockers)
    {
        printf("Iso Blocker: %d %s\n", ics.size(), ics.getCurrentCard(_initialState).getType().getUIName().c_str());
    }

    for (IsomorphicCardSet & ics : _isomorphicChillers)
    {
        printf("Iso Chiller: %d %s\n", ics.size(), ics.getCurrentCard(_initialState).getType().getUIName().c_str());
    }
}

// here we check to see if a given state is the same as, or dominated by any state we've seen before
bool StateChillIterator::checkStateIsomorphism()
{
    for (const StateChillIsomorphism & sci : _allStateIsomorphisms)
    {
        if (sci.dominates(_currentStateIsomorphism))
        {
            return false;
        }
    }

    /*if (std::find(_allStateIsomorphisms.begin(), _allStateIsomorphisms.end(), _currentStateIsomorphism) != _allStateIsomorphisms.end())
    {
        return false;
    }*/

    _allStateIsomorphisms.push_back(_currentStateIsomorphism);
    return true;
}

bool StateChillIterator::checkState(const GameState & state)
{
    for (const GameState & s : _allStates)
    {
        if (state.isIsomorphic(s))
        {
            //printf("\n\nISOMORPHIC STATE DETECTED\n\n");
            return false;
        }
    }

    _allStates.push_back(state);
    return true;
}

const Move & StateChillIterator::getBestMove() const
{
    return _bestMove;
}

// accounts for chiller and blocker isomorphisms
void StateChillIterator::recurse(const size_t currentBlockerIndex)
{
    ++_nodesSearched;

    // the min and max index of the blockers we will look over at this depth
    HealthType blockerIndexMin = (HealthType)currentBlockerIndex;
    HealthType blockerIndexMax = (HealthType)_blockers.size();
        
    if (currentBlockerIndex >= _blockers.size())
    {
        return;
    }

    // if we have partially applied chill to a unit, we must finish freezing that unit before moving on
    // by setting blocker max equal to min + 1 then we're only targeting the partially chilled one
    const Card & firstCard = _currentState.getCardByID(_blockers[blockerIndexMin]);
    if (firstCard.currentChill() > 0 && !firstCard.isFrozen())
    {
        blockerIndexMax = blockerIndexMin + 1;        
    }

    // iterate first through every blocker the enemy has remaining
    for (CardID blockerIndex(blockerIndexMin); blockerIndex < blockerIndexMax; ++blockerIndex)
    {
        const Card & blocker = _currentState.getCardByID(_blockers[blockerIndex]);

        HealthType chillerIndexMin = 0;
        HealthType chillerIndexMax = (HealthType)_chillers.size();

        // iterate over choice of chiller to chill this unit with
        for (CardID chillerIndex(chillerIndexMin); chillerIndex < chillerIndexMax; ++chillerIndex)
        {
            const Card & chiller = _currentState.getCardByID(_chillers[chillerIndex]);
            const Action clickChiller(_player, ActionTypes::USE_ABILITY, chiller.getID());
            const Action chillUnit(_player, ActionTypes::CHILL, chiller.getID(), blocker.getID());

            if (blocker.isFrozen())
            {
                continue;
            }

            // if this action isn't legal, continue on to the next one
            if (_currentState.isLegal(clickChiller))
            {
                _currentState.doAction(clickChiller);
                _actionStack.addAction(clickChiller);

                _currentState.doAction(chillUnit);
                _actionStack.addAction(chillUnit);
            }
            else
            {
                //printf("Not legal: %d chills %d\n", chiller.getID(), blocker.getID());
                continue;
            }

            //printf("%3d - ", _nodesSearched);
            //printStack();

            // do the chill action, this will later be undone after we return from recursing
            //_currentState.doAction(chillUnit);

            // recurse to the next depth passing on the current blocker index
            recurse(blocker.isFrozen() ? 0 : blockerIndex);
            
            const Action chillUnitUndo(_player, ActionTypes::UNDO_USE_ABILITY, chiller.getID());

            _currentState.doAction(chillUnitUndo);
            _actionStack.popAction();
            _actionStack.popAction();
        }
    }    
}

void StateChillIterator::printActions(const Move & actions)
{
    for (size_t i(0); i < actions.size(); ++i)
    {
        const Action & action = actions.getAction(i);
        
        if (action.getType() == ActionTypes::CHILL)
        {
            printf("(%d,%d) ", action.getID(), action.getTargetID());
        }
    }

    printf("\n");
}

void StateChillIterator::printStackEnglish()
{
    for (size_t i(0); i < _actionStack.size(); ++i)
    {
        const Action & action = _actionStack.getAction(i);
        
        if (action.getType() == ActionTypes::CHILL)
        {
            printf("(%s,%s) ", _currentState.getCardByID(action.getID()).getType().getUIName().c_str(), _currentState.getCardByID(action.getTargetID()).getType().getUIName().c_str());
        }
    }

    printf("\n");
}

StateChillEvaluation::StateChillEvaluation() 
    : _damageEvaluation(0)
    , _actionsPerformed(0) 
{
}

StateChillEvaluation::StateChillEvaluation(double e, size_t a) 
    : _damageEvaluation(e), 
    _actionsPerformed(a) 
{
}

bool StateChillEvaluation::betterThan (const StateChillEvaluation & other) const
{
    if (_damageEvaluation == other._damageEvaluation)
    {
        return _actionsPerformed < other._actionsPerformed;
    }

    return _damageEvaluation > other._damageEvaluation;
}

StateChillIsomorphism::StateChillIsomorphism()
{
    std::memset(_blockerCounts, 0, sizeof _blockerCounts);
    std::memset(_chillerCounts, 0, sizeof _chillerCounts);
}

bool StateChillIsomorphism::operator == (const StateChillIsomorphism & other) const
{
    return  blockerCountsEqual(other) && chillerCountsEqual(other);  
}

void StateChillIsomorphism::incBlocker(size_t index)
{
    _blockerCounts[index]++;
}

void StateChillIsomorphism::decBlocker(size_t index)
{
    _blockerCounts[index]--;
}

void StateChillIsomorphism::incChiller(size_t index)
{
    _chillerCounts[index]++;
}

void StateChillIsomorphism::decChiller(size_t index)
{
    _chillerCounts[index]--;
}

bool StateChillIsomorphism::dominates(const StateChillIsomorphism & other) const
{
    if (!blockerCountsEqual(other))
    {
        return false;
    }

    for (size_t i(0); i < 10; ++i)
    {
        if (_chillerCounts[i] > other._chillerCounts[i])
        {
            return false;
        }
    }

    return true;
}

bool StateChillIsomorphism::blockerCountsEqual(const StateChillIsomorphism & other) const
{
    return std::memcmp(_blockerCounts, other._blockerCounts, 10 * sizeof(size_t)) == 0;
}

bool StateChillIsomorphism::chillerCountsEqual(const StateChillIsomorphism & other) const
{
    return std::memcmp(_chillerCounts, other._chillerCounts, 10 * sizeof(size_t)) == 0;
}