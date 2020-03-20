#include "BlockIterator.h"

using namespace Prismata;

BlockIterator::BlockIterator(const GameState & state, EvaluationType (*heuristic)(const Card &, const GameState & state, const HealthType &))
    : _state(state)
    , _player(state.getActivePlayer())
    , _minLossScore(std::numeric_limits<double>::max())
    , _minLossTieBreak(std::numeric_limits<double>::max())
    , _numRecursions(0)
    , _heuristic(heuristic)
{
    _blockers.reserve(20);
    _isoBlockers.reserve(20);
    _bestBlockers.reserve(20);

    for (const auto & cardID : state.getCardIDs(state.getActivePlayer()))
    {
        const Card & card = state.getCardByID(cardID);

        if (card.canBlock())
        {
            _blockers.push_back(&card);
        }
    }

    for (CardID b(0); b < _blockers.size(); ++b)
    {
        int index = findIsomorphicBlockerIndex(_blockers[b]);
        
        if (index != -1)
        {
            _isoBlockers[index].incrementCount();
        }
        else
        {
            _isoBlockers.push_back(IsomorphicBlockerData(*_blockers[b]));
        }
    }

    PRISMATA_ASSERT(state.getActivePhase() == Phases::Defense, "Must be in defense phase to use block iterator");
    PRISMATA_ASSERT(_isoBlockers.size() > 0, "No Isomorphic Blockers Found");
}

void BlockIterator::solve()
{
    recurse(0, _state.getAttack(_state.getEnemy(_state.getActivePlayer())), 0, 0);
}

void BlockIterator::recurse(const CardID & depth, const HealthType & damageRemaining, const double lossScore, const double tieBreakScore)
{
    // solve condition
    if (damageRemaining == 0)
    {
        if (lossScore < _minLossScore)
        {
            _minLossScore = lossScore;
            setBestBlockSequence();
        }
        /*else if (lossScore == _minLossScore && tieBreakScore < _minLossTieBreak)
        {
            _minLossScore = lossScore;
            _minLossTieBreak = tieBreakScore;
            setBestBlockSequence();
        }*/

        return;
    }

    // prune if the current loss score is already higher than our previous best
    if ((lossScore > _minLossScore) || ((lossScore == _minLossScore) && (tieBreakScore >= _minLossTieBreak)))
    {
        return;
    }

    ++_numRecursions;

    // check to see if any card can absorb the remaining damage
    for (CardID b(0); b < _isoBlockers.size(); ++b)
    {
        IsomorphicBlockerData & blocker = _isoBlockers[b];
        if (isLastBlocker(blocker, damageRemaining))
        {
            double loss = _heuristic(blocker.getCard(), _state, damageRemaining);
            double tieBreakLoss = Heuristics::DamageLoss_WillCost(blocker.getCard(), _state, damageRemaining);

            _lastBlocker = blocker;
            blocker.incrementBlocking();
            recurse(depth, 0, lossScore + loss, tieBreakScore + tieBreakLoss);
            blocker.decrementBlocking();

            // if this is depth 0 and we had no loss, we can't do better
            if (depth == 0 && loss == 0)
            {
                return;
            }
        }
    }
    
    // if we can block with this card, do it and recurse with same depth
    IsomorphicBlockerData & blocker = _isoBlockers[depth];
    if (blocker.canBlock())
    {
        HealthType takeDamage = blocker.getHealth() < damageRemaining ? blocker.getHealth() : damageRemaining;
        double loss = _heuristic(blocker.getCard(), _state, damageRemaining);
        double tieBreakLoss = Heuristics::DamageLoss_WillCost(blocker.getCard(), _state, damageRemaining);
        
        blocker.incrementBlocking();
        recurse(depth, damageRemaining - takeDamage, lossScore + loss, tieBreakScore + tieBreakLoss);
        blocker.decrementBlocking();
    }

    // if we have remaining blockers, try them
    if (depth + 1u < _isoBlockers.size())
    {
        recurse(depth + 1, damageRemaining, lossScore, tieBreakScore);
    }
}

void BlockIterator::getBestMove(Move & move)
{
    if (_state.getAttack(_state.getEnemy(_state.getActivePlayer())) == 0)
    {
        return;
    }

    std::vector<bool> blockerUsed(_blockers.size(), false);

    for (size_t b(0); b<_bestBlockers.size(); ++b)
    {
        IsomorphicBlockerData & blocker = _bestBlockers[b];

        // if the best last blocker is isomorphic, decrement the blocking count
        if ((!_bestLastBlocker.isNull()) && _bestLastBlocker.getCard().isIsomorphic(blocker.getCard()))
        {
            blocker.decrementBlocking();
        }

        // while there are any of this card blocking
        while (blocker.getNumBlocking() > 0)
        {
            // get an isomorphic blocker from the blocker list
            // we set the blocker to null if we've used it already
            const Card * card = NULL;
            for (size_t c(0); c<_blockers.size(); ++c)
            {
                if (_blockers[c] != NULL && _blockers[c]->isIsomorphic(blocker.getCard()))
                {
                    card = _blockers[c];
                    _blockers[c] = NULL;
                    break;
                }
            }

            PRISMATA_ASSERT(card != NULL, "We should have found a matching card");

            Action block(card->getPlayer(), ActionTypes::ASSIGN_BLOCKER, card->getID());
            move.addAction(block);

            blocker.decrementBlocking();
        }
    }

    // add the last blocker to the move if necessary
    const Card * card = NULL;
    if (!_bestLastBlocker.isNull())
    {
        for (size_t c(0); c<_blockers.size(); ++c)
        {
            if (_blockers[c] != NULL && _blockers[c]->isIsomorphic(_bestLastBlocker.getCard()))
            {
                card = _blockers[c];
                _blockers[c] = NULL;
                break;
            }
        }
        
        PRISMATA_ASSERT(card != NULL, "We should have found a matching card for the last blocker");

        Action block(card->getPlayer(), ActionTypes::ASSIGN_BLOCKER, card->getID());
        move.addAction(block);
    }
}

double BlockIterator::getMinLossScore() const
{
    return _minLossScore;
}

bool BlockIterator::isLastBlocker(IsomorphicBlockerData & blocker, const HealthType & damageRemaining)
{
    bool canBlock = blocker.getNumBlocking() < blocker.getNum();
    bool isLastBlocker = blocker.getHealth() >= damageRemaining;
    return (canBlock && isLastBlocker);
}

void BlockIterator::setBestBlockSequence()
{
    _bestBlockers = _isoBlockers;
    _bestLastBlocker = _lastBlocker;
}

int BlockIterator::findIsomorphicBlockerIndex(const Card * blocker)
{
    for (CardID b(0); b < _isoBlockers.size(); ++b)
    {
        if (_isoBlockers[b].getCard().isIsomorphic(*blocker))
        {
            return (int)b;
        }
    }

    return -1;
}

void BlockIterator::printIsomorphicBlockers() const
{
    std::cout << std::endl;
    for (CardID b(0); b < _isoBlockers.size(); ++b)
    {
        std::cout << _isoBlockers[b].getNum() << " " << _isoBlockers[b].getCard().getType().getUIName() << " " << _heuristic(_isoBlockers[b].getCard(), _state, 10000) << std::endl;
    }
    std::cout << std::endl;
}