#include "PartialPlayerSequence.h"

using namespace Prismata;

PPSequence::PPSequence()
    : m_sequence(PPPhases::NUM_PHASES)
{

}

const PPPtr & PPSequence::operator [] (const size_t index) const
{
    return m_sequence[index];
}

PPPtr & PPSequence::operator [] (const size_t index)
{
    return m_sequence[index];
}

size_t PPSequence::size() const
{
    return m_sequence.size();
}

void PPSequence::setPP(const size_t index, const PPPtr & partialPlayer)
{
    m_sequence[index] = partialPlayer;
}

std::string PPSequence::getDescription() 
{ 
    std::stringstream ss; 
    
    for (size_t i(0); i<m_sequence.size(); ++i) 
    { 
        ss << m_sequence[i]->getDescription() << "\n"; 
    } 
    
    return ss.str(); 
}

PPSequence PPSequence::clone()
{
    PPSequence newSequence;

    for (size_t i(0); i < PPPhases::NUM_PHASES; ++i)
    {
        newSequence[i] = m_sequence[i]->clone();
    }

    return newSequence;
}