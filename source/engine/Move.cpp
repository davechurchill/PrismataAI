#include "Move.h"

using namespace Prismata;

Move::Move()
{
    m_actions.reserve(40);
}

bool Move::operator == (const Move & rhs) const
{
    for (size_t a(0); a<size(); ++a)
    {
        if (m_actions[a] != rhs.m_actions[a])
        {
            return false;
        }
    }

    return true;
}

bool Move::memEquals(const Move & rhs) const
{
    return m_actions == rhs.m_actions;
}

void Move::set(const Move & move)
{
    m_actions = move.m_actions;
}

void Move::addMove(const Move & move)
{
    for (size_t m(0); m<move.size(); ++m)
    {
        addAction(move.getAction(m));
    }
}

void Move::addAction(const Action & action)
{
    PRISMATA_ASSERT((m_actions.size() == 0) || (action.getPlayer() == m_actions[0].getPlayer()), "All Actions in a Move must have same PlayerID");

    m_actions.push_back(action);
}

const size_t Move::size() const
{
    return m_actions.size();
}

const Action & Move::getAction(const size_t index) const
{
    PRISMATA_ASSERT(index < m_actions.size(), "Trying to get move %d from move of size %d", index, m_actions.size());

    return m_actions[index];
}

const Action & Move::getLastAction() const
{
    return m_actions.back();
}

const std::string Move::toString() const
{
    std::stringstream ss;

    for (size_t m(0); m<size(); ++m)
    {
        ss << getAction(m).toStringEnglish() << std::endl;
    }

    return ss.str();
}

const std::string Move::toClientString() const
{
    std::stringstream ss;

    Action end(getAction(0).getPlayer(), ActionTypes::END_PHASE, 0);
    bool isBreach =  false;

    for (size_t m(0); m<size(); ++m)
    {
        // if this is a non-block and the last action was a block we need to insert a space
        if (m > 0 && getAction(m).getType() != ActionTypes::ASSIGN_BLOCKER && getAction(m-1).getType() == ActionTypes::ASSIGN_BLOCKER)
        {
            ss << end.toClientString() << "\n";
        }

        // if this is a breach and the last action was a non-breach we need to insert a space
        if (m > 0 && getAction(m).getType() == ActionTypes::ASSIGN_BREACH && getAction(m-1).getType() != ActionTypes::ASSIGN_BREACH)
        {
            ss << end.toClientString() << "\n";
            isBreach = true;
        }
        
        // we will insert end phases manually
        if (getAction(m).getType() != ActionTypes::END_PHASE)
        {
            ss << getAction(m).toClientString();
            ss << "\n";
        }
    }

    ss << end.toClientString() << "\n";
    ss << end.toClientString() << "\n";

    return ss.str();
}

void Move::clear()
{
    m_actions.clear();
}

const std::string Move::toHistoryString() const
{
    std::stringstream ss;

    for (size_t a(0); a<m_actions.size(); ++a)
    {
        ss << m_actions[a].toHistoryString() << std::endl;
    }

    return ss.str();
}

void Move::popAction()
{
    m_actions.pop_back();
}