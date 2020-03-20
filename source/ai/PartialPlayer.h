#pragma once

#include "Common.h"
#include "GameState.h"
#include "BuyLimits.h"

namespace Prismata
{

namespace PPPhases
{
    enum { DEFENSE=0, ACTION_ABILITY=1, ACTION_BUY=2, BREACH=3, NUM_PHASES=4 };
}

class PartialPlayer;
typedef std::shared_ptr<PartialPlayer> PPPtr; 

class PartialPlayer 
{

protected:

    PlayerID            _playerID;      // the player this PartialPlayer chooses moves for
    EnumType            _phaseID;       // the phase this PartialPlayer should be choosing moves in
    std::string         _description;
    BuyLimits           _buyLimits;

public:

    virtual void                getMove(GameState & state, Move & move);
    //virtual EnumType            getPhase() const;
    virtual std::string         getDescription(size_t depth = 0);
    virtual const PlayerID      playerID() const;
    virtual void                setDescription(const std::string & description) { _description = description; }
    virtual void                setBuyLimits(const BuyLimits & buyLimits);
    virtual const BuyLimits &   getBuyLimits();

    virtual PPPtr               clone() { return PPPtr(new PartialPlayer(*this));}
};


}