#pragma once

#include "Common.h"
#include "Action.h"

#define MAX_MOVE_ACTIONS 512

namespace Prismata
{
 
class Move
{
    std::vector<Action> m_actions;
    
public:

    Move();

    bool operator == (const Move & rhs) const;
    bool memEquals(const Move & rhs) const;

    void set(const Move & move);
    void addAction(const Action & action);
    void addMove(const Move & move);
    const size_t size() const;
    const Action & getAction(const size_t index) const;
    const Action & getLastAction() const;
    void clear();

    void popAction();
    const std::string toString() const;
    const std::string toClientString() const;
    const std::string toHistoryString() const;
};

}