#include "Action.h"
#include <sstream>

using namespace Prismata;

Action::Action()
{

}

Action::Action(const PlayerID player, const ActionID & actionType, const CardID id)
    : m_type(actionType)
    , m_player(player)
    , m_id(id)
{
  
}

Action::Action(const PlayerID player, const ActionID & actionType, const CardID id, const CardID target)
    : m_type(actionType)
    , m_player(player)
    , m_id(id)
    , m_targetID(target)
{
    
}

bool Action::operator == (const Action & rhs) const
{
    return (m_type == rhs.m_type) && (m_id == rhs.m_id) && (m_player == rhs.m_player);
}

bool Action::operator != (const Action & rhs) const
{
    return !(*this == rhs);
}
    
ActionID Action::getType() const
{
    return m_type;
}

CardID Action::getID() const
{
    return m_id;
}

void Action::setID(const CardID id)
{
    m_id = id;
}

CardID Action::getTargetID() const
{
    return m_targetID;
}

PlayerID Action::getPlayer() const
{
    return m_player;
}

std::string Action::toString() const
{
    std::stringstream ss;
    ss << "(Player = " << (int)getPlayer() << ", Type = " << (int)getType() << ", ID = " << (int)getID() << ", Target = " << (int)getTargetID() <<")";
    return ss.str();
}

void Action::setShift(bool shift)
{
    m_shift = shift;
}

bool Action::getShift() const
{
    return m_shift;
}

std::string Action::toStringEnglish() const
{
    std::stringstream ss;
    ss << "Player " << (int)getPlayer() << " ";
    switch (getType())
    {
        case ActionTypes::ASSIGN_BLOCKER:   { ss << "Blocks with Card " << (int)getID(); break; }
        case ActionTypes::ASSIGN_BREACH:    { ss << "Breaches Card " << (int)getID(); break; }
        case ActionTypes::BUY:              { ss << "Buys Card " << (int)getID(); break; }
        case ActionTypes::END_PHASE:        { ss << "Ends Phase " << (int)getID(); break; }
        case ActionTypes::USE_ABILITY:      { ss << "Uses Ability of Card " << (int)getID(); break; }
        case ActionTypes::CHILL:            { ss << "Chills card " << (int)getID() << " with card " << (int)getTargetID(); break; }
        case ActionTypes::UNDO_CHILL:       { ss << "Undo Chill " << (int)getID(); break; }
        default:                            { ss << "Unknown Action"; break; }
    }
    
    return ss.str();
}

std::string Action::toClientString() const
{
    std::stringstream ss;

    if (getType() == ActionTypes::ASSIGN_BLOCKER)
    {
        ss << "Block " << (int)ClickTypes::BeginSwipe << " " << (int)getID() << "\n";
        ss << "Block " << (int)ClickTypes::EndSwipe << " " << (int)getID();
    }
    else if (getType() == ActionTypes::ASSIGN_BREACH)
    {
        ss << "Breach " << (int)ClickTypes::BeginSwipe << " " << (int)getID() << "\n";
        ss << "Breach " << (int)ClickTypes::EndSwipe << " " << (int)getID();
    }
    else if (getType() == ActionTypes::BUY)
    {
        ss << "Buy " << (int)ClickTypes::Card << " "  << (int)getID();
    }
    else if (getType() == ActionTypes::END_PHASE)
    {
        ss << "Space " << (int)ClickTypes::Space << " " << -1;
    }
    else if (getType() == ActionTypes::USE_ABILITY)
    {
        ss << "Ability " << (int)ClickTypes::BeginSwipe << " " << (int)getID() << "\n";
        ss << "Ability " << (int)ClickTypes::EndSwipe << " " << (int)getID();
    }
    else if (getType() == ActionTypes::UNDO_USE_ABILITY)
    {
        ss << "Undo Ability " << (int)ClickTypes::BeginSwipe << " " << (int)getID() << "\n";
        ss << "Undo Ability " << (int)ClickTypes::EndSwipe << " " << (int)getID();
    }
    else if (getType() == ActionTypes::UNDO_CHILL)
    {
        ss << "Undo Chill " << (int)ClickTypes::BeginSwipe << " " << (int)getID() << "\n";
        ss << "Undo Chill " << (int)ClickTypes::EndSwipe << " " << (int)getID();
    }
    
    return ss.str();
}

std::string Action::toStringEnglishShort() const
{
    std::stringstream ss;

    if (getType() == ActionTypes::ASSIGN_BLOCKER)
    {
        ss << "Block " << (int)getID();
    }
    else if (getType() == ActionTypes::ASSIGN_BREACH)
    {
        ss << "Breach " << (int)getID();
    }
    else if (getType() == ActionTypes::BUY)
    {
        ss << "Buy " << (int)getID();
    }
    else if (getType() == ActionTypes::END_PHASE)
    {
        ss << "End Phase";
    }
    else if (getType() == ActionTypes::USE_ABILITY)
    {
        ss << "Ability " << (int)getID();
    }
    
    return ss.str();
}

std::string Action::toHistoryString() const
{
    std::stringstream ss;

    ss << (int)getPlayer() << " " << (int)getType() << " " << (int)getID() << " " << (int)getTargetID();

    return ss.str();
}
