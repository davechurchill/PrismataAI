#pragma once

#include <stddef.h>

// type definitions for storing data
namespace Prismata
{
    typedef unsigned char   PlayerID;           
    typedef unsigned char   ActionID;   
    
    typedef size_t          CardID;  
    typedef CardID          SupplyType;
    typedef unsigned short  ChargeType;   
    typedef unsigned short  HealthType;
    typedef unsigned short  TurnType;
    typedef unsigned short  ResourceType;

    typedef double          EvaluationType;
    typedef double          UCTValue;
    typedef double          AlphaBetaValue;
    typedef double          StateEvalScore;
}

