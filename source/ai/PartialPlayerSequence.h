#pragma once

#include "Common.h"
#include "GameState.h"
#include "PartialPlayer.h"

namespace Prismata
{
   
class PPSequence
{
    std::vector<PPPtr> m_sequence;

public:

    PPSequence();

    const PPPtr & operator [] (const size_t index) const;
    PPPtr & operator [] (const size_t index);

    void setPP(const size_t index, const PPPtr & partialPlayer);
    size_t size() const;

    std::string getDescription();

    PPSequence clone();
};

}