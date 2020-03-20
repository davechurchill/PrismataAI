#pragma once

#include "Common.h"
#include "GameState.h"

#include "MoveIterator.h"
#include "Player.h"
#include "PartialPlayer.h"
#include "PartialPlayerSequence.h"

namespace Prismata
{

class MoveIterator_PPPortfolio : public MoveIterator
{

protected:

    std::vector<std::vector<PPPtr>> m_portfolio;             // portfolio of partial players for each phase of the game
    std::vector<size_t>             m_currentIndex;          // the current indices of partial players in the portfolio
    PPSequence                      m_currentSequence;       // the current sequence to use as a Player_PPSequence
    PPSequence                      m_previousSequence;
    std::vector<Move>               m_movesPerformed;
    std::vector<bool>               m_previousMoveChanged;
    std::vector<Move>               m_previousMoveGenerated;
    bool                            m_hasDescription = false;
    
    void            incrementMove(const size_t phase = PPPhases::NUM_PHASES-1);

public:

    MoveIterator_PPPortfolio(const PlayerID & playerID);

    bool            generateNextChild(GameState & child, Move & movePerformed);
    void            addPartialPlayer(const size_t phase, const PPPtr & player);
    void            reset();
    void            setState(const GameState & state);
    void            getRandomMove(const GameState & state, Move & move);
    void            setBuyLimits(const BuyLimits & buyLimits);
    std::string     getDescription();
    MoveIteratorPtr clone();

};

}