#pragma once

#include "Common.h"

namespace Prismata
{

namespace ActionTypes
{
    enum
    {
        // forward abilities
        USE_ABILITY,
        BUY,
        END_PHASE,
        ASSIGN_BLOCKER,
        ASSIGN_BREACH,
        ASSIGN_FRONTLINE,
        SNIPE,
        CHILL,
        WIPEOUT,

        // reverse abilities
        UNDO_USE_ABILITY,
        UNDO_CHILL,
        UNDO_BREACH,
        SELL,

        // book keeping
        NUM_TYPES,
        NONE
    };
}

class Action
{
    PlayerID    m_player    = (PlayerID)-1;     // the player issuing the action
    ActionID    m_type      = ActionTypes::NONE;// the type of action this is
    CardID      m_id        = (CardID)-1;       // USE_ABILITY:     ID of the Card using the ability
                                                // BUY:        Index of the CardBuyable in GameStateCardBuyableData
                                                // ASSIGN_BLOCKER:  ID of the Card being used to block
                                                // ASSIGN_BREACH:   ID of the Crd to assign breach to
    CardID      m_targetID  = 0;                // the CardID of the target of this action
    bool        m_shift     = false;            // shift click?

public:

    Action();
    Action(const PlayerID player, const ActionID & actionType, const CardID id = 0);
    Action(const PlayerID player, const ActionID & actionType, const CardID id, const CardID target);

    void setShift(bool shift);
    void setID(const CardID id);
    bool getShift() const;

    bool operator == (const Action & rhs)   const;
    bool operator != (const Action & rhs)   const;

    ActionID     getType()                  const;
    CardID       getID()                    const;
    PlayerID     getPlayer()                const;
    CardID       getTargetID()              const;

    std::string   toString()                const;
    std::string   toStringEnglish()         const;
    std::string   toStringEnglishShort()    const;
    std::string   toClientString()          const;
    std::string   toHistoryString()         const;
};

}