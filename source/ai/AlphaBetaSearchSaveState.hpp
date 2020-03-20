#pragma once

#include "Common.h"
#include "Player.h"
#include "MoveIterator.h"

namespace Prismata
{
    
class AlphaBetaSearchSaveState
{                   
public:

    std::vector<size_t>             variation;
    std::vector<StateEvalScore>     alphas;
    std::vector<StateEvalScore>     betas;
    size_t                          previousMaxDepth;
    double                          previousTimeElapsed;

    AlphaBetaSearchSaveState()
        : previousMaxDepth(0)
        , previousTimeElapsed(0)
    {
    
    }

    const std::string getDescription() const
    {
        std::stringstream ss;
        //ss << "previousMaxDepth           = " << previousMaxDepth << "\n";
        //ss << "previousAlpha              = " << previousAlpha << "\n";
        //ss << "previousBeta               = " << previousBeta << "\n";
        //ss << "previousTimeElapsed        = " << previousTimeElapsed << "\n";

        for (size_t i(0); i<variation.size(); ++i)
        {
            ss << variation[i] << " ";
        }

        return ss.str();
    }
};
}