#pragma once

#include "Common.h"

namespace Prismata
{
class AlphaBetaSearchResults
{

public:

    bool                        searchCompleted;   // whether ot not a solution was found
    bool                        timedOut;  // did the search time-out?
    bool                        searchInProgress;
 
    unsigned long long          nodesExpanded; // number of nodes expanded in the search
    unsigned long long          actionsPerformed;
    unsigned long long          evaluationsPerformed;
 
    std::vector<double>         timeElapsed; // time elapsed in milliseconds
    double                      totalTimeElapsed;

    AlphaBetaSearchSaveState    saveState;
    size_t                      maxDepthCompleted;
    std::vector<Move>           bestMoves;
    std::vector<double>         bestMoveValues;
    std::vector<std::string>    bestMoveDescs;
    int                         rootNumChildren;
    int                         rootCurrentChild;
    double                      currentAlpha;
    double                      currentBeta;
    size_t                      nodesCompleted;
    size_t                      branches;

    AlphaBetaSearchResults() 
        : searchCompleted(false)
        , timedOut(false)
        , searchInProgress(false)
        , nodesExpanded(0)
        , evaluationsPerformed(0)
        , timeElapsed(0)
        , totalTimeElapsed(0)
        , actionsPerformed(0)
        , maxDepthCompleted(0)
        , rootNumChildren(0)
        , rootCurrentChild(0)
        , currentAlpha(0)
        , currentBeta(0)
        , nodesCompleted(0)
        , branches(0)
    {
    }

    void setMaxDepth(size_t maxDepth)
    {
        bestMoves        = std::vector<Move>(maxDepth + 2);
        bestMoveValues   = std::vector<double>(maxDepth + 2, -100000);
        bestMoveDescs    = std::vector<std::string>(maxDepth + 2, "");
        timeElapsed      = std::vector<double>(maxDepth + 2, 0);
    }
        
    Move & bestMove()
    {
        return bestMoves[maxDepthCompleted];
    }

    std::string getDescription(size_t currentDepth)
    {
        std::stringstream ss;
        ss << "";

        ss << "\nAlphaBeta Results\n";
        ss << "Cur Max Depth:  " << currentDepth << "\n";
        ss << "Time Elapsed:   " << totalTimeElapsed << "ms\n";
        ss << "Nodes Expanded: " << nodesExpanded << "\n";
        ss << "Nodes / Second: " << (int)((double)nodesExpanded / (totalTimeElapsed+1) * 1000) << "\n";
        ss << "Evals / Second: " << (int)((double)evaluationsPerformed / (totalTimeElapsed+1) * 1000) << "\n";
        //ss << "Avg Branching:  " << ((double)branches / (double)nodesCompleted) << "\n\n";
        //ss << "Total Actions:  " << actionsPerformed << "\n";
        //ss << "Best Value:     " << bestMoveValues[currentDepth] << "\n";
        //ss << "Alpha:          " << currentAlpha << "\n";
        //ss << "Beta:           " << currentBeta << "\n";
        //ss << "Window Size:    " << (currentBeta-currentAlpha) << "\n";
        //ss << "Root Child:     " << rootCurrentChild << "\n\n";
        ss << "\n" << bestMoveDescs[currentDepth];

        return ss.str();
    }

};
}