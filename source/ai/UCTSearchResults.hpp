#pragma once

#include "Common.h"

namespace Prismata
{
class UCTSearchResults
{

public:

    unsigned long long          nodesExpanded; // number of nodes expanded in the search
    double                      timeElapsed; // time elapsed in milliseconds

    size_t                      traversals;
    size_t                      traverseCalls;
    size_t                      nodesVisited;
    size_t                      totalVisits;
    size_t                      nodesCreated;
    size_t                      treeSize;

    std::string                 bestMoveDescription;

    Move                        bestMoves;

     UCTSearchResults() 
        : nodesExpanded         (0)
        , timeElapsed           (0)
        , traversals            (0)
        , traverseCalls         (0)
        , nodesVisited          (0)
        , totalVisits           (0)
        , nodesCreated          (0)
        , treeSize              (0)
     {
     }

    std::string getDescription()
    {
        std::stringstream ss;

        ss << "\nUCT Results\n";
        ss << "Time Elapsed:   " << timeElapsed << "ms\n";
        ss << "Traversals:     " << traversals << "\n";
        ss << "Traversals / s: " << (traversals / (timeElapsed+1)) * 1000 << "\n";
        ss << "Node Visits:    " << totalVisits << "\n";
        ss << "Nodes Created:  " << nodesCreated << "\n";
        ss << "Tree Size:      " << treeSize/1024 << "kb\n";
        ss << "\n" << bestMoveDescription;

        return ss.str();
    }
};
}