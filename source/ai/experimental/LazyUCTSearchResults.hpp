#pragma once

#include "Common.h"

namespace Prismata
{

class LazyUCTSearchResults
{
public:

    double              timeElapsed = 0;
    size_t              traversals = 0;
    size_t              traverseCalls = 0;
    size_t              evaluations = 0;
    size_t              nodesCreated = 0;
    size_t              nodesVisited = 0;
    size_t              totalVisits = 0;
    size_t              rootChildren = 0;
    size_t              maxDepthReached = 0;
    double              bestMoveValue = 0;
    std::string         bestMoveDescription;

    std::string getDescription() const
    {
        std::stringstream ss;

        ss << "\nLazy UCT Results\n";
        ss << "Time Elapsed:   " << timeElapsed << "ms\n";
        ss << "Traversals:     " << traversals << "\n";
        ss << "Traversals / s: " << (traversals / (timeElapsed + 1)) * 1000 << "\n";
        ss << "Calls:          " << traverseCalls << "\n";
        ss << "Evaluations:    " << evaluations << "\n";
        ss << "Node Visits:    " << totalVisits << "\n";
        ss << "Nodes Created:  " << nodesCreated << "\n";
        ss << "Root Children:  " << rootChildren << "\n";
        ss << "Max Depth:      " << maxDepthReached << "\n";
        ss << "Best Value:     " << bestMoveValue << "\n";
        ss << "\n" << bestMoveDescription;

        return ss.str();
    }
};

}
