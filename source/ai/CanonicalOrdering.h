#pragma once

#include "Common.h"
#include "CardType.h"

namespace Prismata
{

class CanonicalOrdering
{
    std::vector< std::vector<size_t> >  _orderingGraph;
    std::vector< size_t >               _ordering;  

    bool canonicalGreaterThan(size_t i, size_t j);
    bool canonicalLessThan(size_t i, size_t j);

public:

    CanonicalOrdering();
    void createGraph(const std::vector<CardType> & types);

    void print();
};
}