#include "CanonicalOrdering.h"

using namespace Prismata;

CanonicalOrdering::CanonicalOrdering()
    : _orderingGraph(Resources::NumTypes+1, std::vector<size_t>(Resources::NumTypes+1, 0))
{
    for (size_t i(0); i < Resources::NumTypes+1; ++i)
    {
        _ordering.push_back(i);
    }
}

void CanonicalOrdering::createGraph(const std::vector<CardType> & types)
{
    for (size_t i(0); i < types.size(); ++i)
    {
        const CardType type = types[i];

        bool sacsUnit = type.getAbilityScript().getSacCost().size() > 0;
        const Resources & consumed = type.getAbilityScript().getManaCost();
        const Resources & produced = type.getAbilityScript().getEffect().getReceive();

        if (consumed.empty() && produced.empty())
        {
            continue;
        }

        // add the sac > resource relationships
        if (sacsUnit)
        {
            for (size_t p(0); p < Resources::NumTypes; ++p)
            {
                if (produced.amountOf(p) > 0)
                {
                    //printf("%20s %c > %c\n", type.getUIName().c_str(), Resources::CharReal[Resources::Sac], Resources::CharReal[p]);
                    _orderingGraph[Resources::Sac][p]++;
                }
            }
        }

        // add the resource > resource relationships
        for (size_t c(0); c < Resources::NumTypes; ++c)
        {
            if (consumed.amountOf(c) > 0)
            {
                for (size_t p(0); p < Resources::NumTypes; ++p)
                {
                    if (produced.amountOf(p) > 0)
                    {
                        //printf("%20s %c > %c\n", type.getUIName().c_str(), Resources::CharReal[c], Resources::CharReal[p]);
                        _orderingGraph[c][p]++;
                    }
                }
            }
        }
    }

    for (size_t i(0); i < _ordering.size()-1; ++i)
    {
        for (size_t j(i+1); j < _ordering.size(); ++j)
        {
            if (canonicalLessThan(_ordering[i], _ordering[j]))
            {
                std::swap(_ordering[i], _ordering[j]);
            }
        }
    }


    //print();
}

bool CanonicalOrdering::canonicalGreaterThan(size_t i, size_t j)
{
    return _orderingGraph[i][j] > 0;
}

bool CanonicalOrdering::canonicalLessThan(size_t i, size_t j)
{
    return _orderingGraph[j][i] > 0;
}

void CanonicalOrdering::print()
{
    printf("\n   ");
    for (size_t i(0); i < _orderingGraph.size(); ++i)
    {
        printf("%3c", Resources::GetCharReal(i));
    }

    printf("\n");

    for (size_t i(0); i < _orderingGraph.size(); ++i)
    {
        printf("%3c", Resources::GetCharReal(i));

        for (size_t j(0); j < _orderingGraph[i].size(); ++j)
        {
            printf("%3d", _orderingGraph[i][j]);
        }

        printf("\n");
    }


    printf("\nCanonical Ordering: %c", Resources::GetCharReal(_ordering[0]));

    for (size_t i(1); i < _ordering.size(); ++i)
    {
        if (canonicalGreaterThan(_ordering[i-1], _ordering[i]))
        {
            printf(" > ");
        }

        printf("%c", Resources::GetCharReal(_ordering[i]));
    }

    printf("\n");
}