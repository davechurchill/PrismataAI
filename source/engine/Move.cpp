#include "Move.h"

using namespace Prismata;

Move::Move()
{
    _actions.reserve(40);
}

bool Move::operator == (const Move & rhs) const
{
    for (size_t a(0); a<size(); ++a)
    {
        if (_actions[a] != rhs._actions[a])
        {
            return false;
        }
    }

    return true;
}

bool Move::memEquals(const Move & rhs) const
{
    return _actions == rhs._actions;
}

void Move::set(const Move & move)
{
    _actions = move._actions;
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
    PRISMATA_ASSERT((_actions.size() == 0) || (action.getPlayer() == _actions[0].getPlayer()), "All Actions in a Move must have same PlayerID");

    _actions.push_back(action);
}

const size_t Move::size() const
{
    return _actions.size();
}

const Action & Move::getAction(const size_t index) const
{
    PRISMATA_ASSERT(index < _actions.size(), "Trying to get move %d from move of size %d", index, _actions.size());

    return _actions[index];
}

const Action & Move::getLastAction() const
{
    return _actions.back();
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
    _actions.clear();
}

const std::string Move::toHistoryString() const
{
    std::stringstream ss;

    for (size_t a(0); a<_actions.size(); ++a)
    {
        ss << _actions[a].toHistoryString() << std::endl;
    }

    return ss.str();
}

void Move::popAction()
{
    _actions.pop_back();
}