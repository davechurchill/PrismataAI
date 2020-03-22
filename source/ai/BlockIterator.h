#pragma once

#include "Common.h"
#include "GameState.h"

#include "BuyLimits.h"
#include "Heuristics.h"
#include "Timer.h"

namespace Prismata
{

class IsomorphicBlockerData
{
    const Card *    _card;
    CardID          _num;
    CardID          _numBlocking;
    
public:

    IsomorphicBlockerData()
        : _card(NULL)
        , _num(0)
        , _numBlocking(0)
    {
    
    }

    IsomorphicBlockerData(const Card & card)
        : _card(&card)
        , _num(1)
        , _numBlocking(0)
    {
        
    }

    const Card & getCard() const
    {
        return *_card;
    }

    const CardID getNum() const
    {
        return _num;
    }

    const CardID getNumBlocking() const
    {
        return _numBlocking;
    }

    const HealthType & getHealth() const
    {
        return getCard().currentHealth();
    }

    bool isIsomorphic(const Card & card) const
    {
        return getCard().isIsomorphic(card);
    }

    bool canBlock() const
    {
        return getNumBlocking() < getNum();
    }

    void incrementCount()
    {
        _num++;
    }

    void incrementBlocking() 
    {
        ++_numBlocking;
    }

    void decrementBlocking() 
    {
        --_numBlocking;
    }

    bool isNull()
    {
        return _card == NULL;
    }
};
 
class BlockIterator 
{
    const GameState &                       _state;
    PlayerID                                _player;   
    EvaluationType                          (*_heuristic)(const Card &, const GameState & state, const HealthType &);
    
    std::vector<const Card *>               _blockers;
    std::vector<IsomorphicBlockerData>      _isoBlockers;
    IsomorphicBlockerData                   _lastBlocker;

    std::vector<IsomorphicBlockerData>      _bestBlockers;
    IsomorphicBlockerData                   _bestLastBlocker;
    
    unsigned long long                      _numRecursions;
    double                                  _minLossScore;
    double                                  _minLossTieBreak;

    int                                     findIsomorphicBlockerIndex(const Card * blocker);
    void                                    setBestBlockSequence();
    bool                                    isLastBlocker(IsomorphicBlockerData & blocker, const HealthType & damageRemaining);

public:
    
    BlockIterator(const GameState & state, EvaluationType (*heuristic)(const Card &, const GameState & state, const HealthType &) = &Heuristics::DamageLoss_WillCost);

    void        solve();
    void        getBestMove(Move & move);
    void        recurse(const CardID depth, const HealthType & damageRemaining, const double lossScore, const double tieBreakScore);
    size_t      getNumBuys();
    double      getMinLossScore() const;

    void        printStack() const;
    void        printIsomorphicBlockers() const;
};


}